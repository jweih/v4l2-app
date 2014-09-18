/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

               Camera    I n t e r f a c e    M O D U L E

                        EDIT HISTORY FOR MODULE

when        who       what, where, why
--------    ---       -------------------------------------------------------
10/xx/08   ZzaU      Created file.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/


/*===========================================================================

                           INCLUDE FILES FOR MODULE

===========================================================================*/
#include "main.h"

#define PANEL_WIDTH "/sys/devices/virtual/tcc_dispman/tcc_dispman/tcc_output_panel_width"
#define PANEL_HEIGHT "/sys/devices/virtual/tcc_dispman/tcc_dispman/tcc_output_panel_height"

/*===========================================================================
FUNCTION
===========================================================================*/
void rsc_init_lcd(CameraDevice* self)
{
	FILE *sysfs_fd = NULL;
	int panel_width, panel_height;
    static struct fb_var_screeninfo fb_info;

	/* read LCD panel size
	 */
	sysfs_fd = fopen(PANEL_WIDTH, "r");
	if (sysfs_fd <= 0) {
		printf("error: open %s\n", PANEL_WIDTH);
		panel_width = 0;
	} else {
		fscanf(sysfs_fd, "%d", &panel_width);
	}
	fclose(sysfs_fd);
	sysfs_fd = fopen(PANEL_HEIGHT, "r");
	if (sysfs_fd <= 0) {
		printf("error: open %s\n", PANEL_HEIGHT);
		panel_height = 0;
	} else {
		fscanf(sysfs_fd, "%d", &panel_height);
	}
	fclose(sysfs_fd);

	/* read fbdev info
	 */
	ioctl(self->fb_fd0, FBIOGET_VSCREENINFO, &fb_info);

	/* set preview size
	 */
	self->preview_width = (panel_width > 0) ? panel_width : fb_info.xres;
	self->preview_height = (panel_height > 0) ? panel_height : fb_info.yres;

	printf("[%s] Overlay size\n", __func__);
	printf("- get output panel size: %dx%d\n", panel_width, panel_height);
	printf("- get framebuffer size : %dx%d\n", fb_info.xres, fb_info.yres);
	printf("- set preview size     : %dx%d\n", self->preview_width, self->preview_height);
}

/*===========================================================================
FUNCTION
===========================================================================*/
void rsc_overlay_ctrl(CameraDevice* self, unsigned char en)
{
	if (en) {
	} else {
		if (self->use_vout == 0) {
			if(self->overlay_fd >= 0)
				ioctl(self->overlay_fd, OVERLAY_SET_DISABLE,NULL);
		}
	}
}

/*===========================================================================
FUNCTION
===========================================================================*/
void rsc_v4l2_qbuf(CameraDevice* self, unsigned int addr)
{
	unsigned int base[3] = {0, 0, 0};
	unsigned int width, height;

	//width = self->preview_width;
	//height = self->preview_height;
	if (self->use_vout == 0) {
		width = self->overlay_config.width;
		height = self->overlay_config.height;
	} else {
		width = self->vout_fmt.fmt.pix.width;
		height = self->vout_fmt.fmt.pix.height;
	}

	switch (self->preview_fmt) {
	case TCC_LCDC_IMG_FMT_YUV420SP:
		base[0] = addr;
		base[1] = base[0] + (width * height);
		base[2] = base[1] + (width * height / 4);
		self->outbuf = (void *)base[0];
		self->mem_len = (width * height * 3) / 2;
		break;
	case TCC_LCDC_IMG_FMT_YUV420ITL0:
	case TCC_LCDC_IMG_FMT_YUV420ITL1:
		base[0] = addr;
		base[1] = base[0] + (width * height);
		base[2] = base[1];
		self->outbuf = (void *)base[0];
		self->mem_len = (width * height * 3) / 2;
		break;
	case TCC_LCDC_IMG_FMT_YUV422SP:
		base[0] = addr;
		base[1] = base[0] + (width * height);
		base[2] = base[1] + (width * height / 2);
		self->outbuf = (void *)base[0];
		self->mem_len = width * height * 2;
		break;
	case TCC_LCDC_IMG_FMT_YUV422ITL0:
	case TCC_LCDC_IMG_FMT_YUV422ITL1:
		base[0] = addr;
		base[1] = base[0] + (width * height);
		base[2] = base[1];
		self->outbuf = (void *)base[0];
		self->mem_len = width * height * 2;
	    break;
	case TCC_LCDC_IMG_FMT_UYVY:
	case TCC_LCDC_IMG_FMT_VYUY:
	case TCC_LCDC_IMG_FMT_YUYV:
	case TCC_LCDC_IMG_FMT_YVYU:
		base[0] = addr;
		self->outbuf = (void *)base[0];
		self->mem_len = width * height * 2;
		break;
	case TCC_LCDC_IMG_FMT_RGB888_3:
		base[0] = addr;
		self->outbuf = (void *)base[0];
		self->mem_len = width * height * 3;	// RGB888 bpp = 3
		break;
	}

	if (self->use_vout == 0) {
		ioctl(self->overlay_fd, OVERLAY_QUEUE_BUFFER,base);
	} else {
		int ret;
	
		//self->vout_qbuf.index;									// already seted by camif_get_frame()
		self->vout_qbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		self->vout_qbuf.field = V4L2_FIELD_NONE;					// progressive
		self->vout_qbuf.timecode.type = 0;							// 0: video, 0xff00: still image 
		self->vout_qbuf.memory = V4L2_MEMORY_USERPTR;
		self->vout_qbuf.m.planes->bytesused = 0x0;					// [16:0] Y-stride, [31:17] UV-stride
		self->vout_qbuf.m.planes->m.userptr = base[0];				// base0 address
		self->vout_qbuf.m.planes->reserved[VID_BASE1] = base[1];	// base1 address
		self->vout_qbuf.m.planes->reserved[VID_BASE2] = base[2];	// base2 address
		self->vout_qbuf.m.planes->reserved[VID_SRC] = MPLANE_VID;
		self->vout_qbuf.m.planes->reserved[VID_NUM] = 1;

		self->vout_qbuf.m.planes->reserved[VID_WIDTH] = self->vout_fmt.fmt.pix.width;
		self->vout_qbuf.m.planes->reserved[VID_HEIGHT] = self->vout_fmt.fmt.pix.height;
		self->vout_qbuf.m.planes->reserved[VID_CROP_LEFT] = 0;
		self->vout_qbuf.m.planes->reserved[VID_CROP_TOP] = 0;
		self->vout_qbuf.m.planes->reserved[VID_CROP_WIDTH] = self->vout_qbuf.m.planes->reserved[VID_WIDTH];
		self->vout_qbuf.m.planes->reserved[VID_CROP_HEIGHT] = self->vout_qbuf.m.planes->reserved[VID_HEIGHT];

		ret = ioctl(self->vout_fd, VIDIOC_QBUF, &self->vout_qbuf);
		if (ret) {
			printf("error: VIDIOC_QBUF(V4L2_BUF_TYPE_VIDEO_OUTPUT)\n");
		}
	}
}

/*===========================================================================
FUNCTION
===========================================================================*/
typedef unsigned long long	mtime_t;
mtime_t imdate(void)
{
	struct timeval tv_date;

	gettimeofday(&tv_date, NULL);

	// exception
	if(tv_date.tv_sec < 0)
{
		tv_date.tv_sec = tv_date.tv_sec * (-1);
		settimeofday(&tv_date, NULL);
		gettimeofday(&tv_date, NULL);
	}

	return (mtime_t)tv_date.tv_sec * 1000000 + (mtime_t)tv_date.tv_usec;
}

/*===========================================================================
FUNCTION
===========================================================================*/
void rsc_buf_timestamp_logprint(struct v4l2_buffer *pBuf)
{
	struct tm *plocal_time;

	plocal_time = (struct tm*)localtime((time_t*)&pBuf->timestamp.tv_sec);
	printf("dqbuf idx = %d, timestamp = %ld:%06ld (%04d-%02d-%02d %02d:%02d:%02d.%06d)\n", 
			   pBuf->index, 
			   pBuf->timestamp.tv_sec, 
			   pBuf->timestamp.tv_usec,
			   plocal_time->tm_year+1900, 
			   plocal_time->tm_mon+1, 
			   plocal_time->tm_mday, 
			   plocal_time->tm_hour, 
			   plocal_time->tm_min, 
			   plocal_time->tm_sec, 
			   (int)pBuf->timestamp.tv_usec	 // /1000
	);
}

void _get_plane_addrs(unsigned int fmt, unsigned int w, unsigned int h, 
					unsigned int *addr_y, unsigned int *addr_u, unsigned int *addr_v)
{
	unsigned int base[3] = {0, 0, 0};
	unsigned int width, height;
	unsigned int addr;

	addr = *addr_y;
	width = w;
	height = h;

	switch (fmt) {
	case TCC_LCDC_IMG_FMT_YUV420SP:
		base[0] = addr;
		base[1] = base[0] + (width * height);
		base[2] = base[1] + (width * height / 4);
		break;
	case TCC_LCDC_IMG_FMT_YUV420ITL0:
	case TCC_LCDC_IMG_FMT_YUV420ITL1:
		base[0] = addr;
		base[1] = base[0] + (width * height);
		base[2] = base[1];
		break;
	case TCC_LCDC_IMG_FMT_YUV422SP:
		base[0] = addr;
		base[1] = base[0] + (width * height);
		base[2] = base[1] + (width * height / 2);
		break;
	case TCC_LCDC_IMG_FMT_YUV422ITL0:
	case TCC_LCDC_IMG_FMT_YUV422ITL1:
		base[0] = addr;
		base[1] = base[0] + (width * height);
		base[2] = base[1];
	    break;
	case TCC_LCDC_IMG_FMT_UYVY:
	case TCC_LCDC_IMG_FMT_VYUY:
	case TCC_LCDC_IMG_FMT_YUYV:
	case TCC_LCDC_IMG_FMT_YVYU:
	default:
		base[0] = addr;
		base[1] = addr;
		base[2] = addr;
		break;
	}

	*addr_y = base[0];
	*addr_u = base[1];
	*addr_v = base[2];
}

/*===========================================================================
FUNCTION
===========================================================================*/
void rsc_directly_draw_lcd(CameraDevice* self, struct v4l2_buffer *pBuf)
{
	/* update overlay configuration.
	 * because we can display different size output.
	 */
	if (self->use_vout == 0) {
		ioctl(self->overlay_fd, OVERLAY_SET_CONFIGURE, &self->overlay_config);
	}

	/* display
	 */
	if (pBuf->m.offset < self->pmap_camera.base)
		rsc_v4l2_qbuf(self, pBuf->m.offset + self->pmap_camera.base);
	else
		rsc_v4l2_qbuf(self, pBuf->m.offset);
}

/*===========================================================================
FUNCTION
===========================================================================*/
void rsc_draw_lcd(CameraDevice* self)
{
}

/*===========================================================================
FUNCTION
===========================================================================*/
void rsc_process_image(CameraDevice *self, uint8_t *source)
{
}
	
/*===========================================================================
FUNCTION
===========================================================================*/
void  rsc_encode_jpeg(CameraDevice *self)
	{
	}
	
/*===========================================================================
FUNCTION
===========================================================================*/
void  rsc_save_file(CameraDevice *self, unsigned short* filename)
{
	
}


/*===========================================================================
FUNCTION
===========================================================================*/
void rsc_sched_delay(int ms)
{
	usleep(ms*1000);
}

void rsc_sched_delay1(int ms)
{
	usleep(ms*1000);
}

