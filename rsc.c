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
#include "tcc_scaler_interface.h"

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

	if (self->outdisp_dev) {
		self->outdisp_info.enable		= 1;
		self->outdisp_info.Lcdc_layer	= LCDC_LAYER_2;
		self->outdisp_info.fmt			= self->preview_fmt;

		self->outdisp_info.offset_x		= fb_info.xoffset;
		self->outdisp_info.offset_y		= fb_info.yoffset;

		self->outdisp_info.Frame_width	= self->vid_fmt.fmt.pix.width;
		self->outdisp_info.Frame_height	= self->vid_fmt.fmt.pix.height;

		self->outdisp_info.crop_left	= 0;
		self->outdisp_info.crop_top		= 0;
		self->outdisp_info.crop_right	= self->output_width;
		self->outdisp_info.crop_bottom	= self->output_height;

		self->outdisp_info.Image_width	= self->vid_fmt.fmt.pix.width;
		self->outdisp_info.Image_height	= self->vid_fmt.fmt.pix.height;
		
		self->outdisp_info.outputMode	= OUTPUT_COMPOSITE;
		self->outdisp_info.on_the_fly	= 0;
	}

	//if (self->outdisp_dev) {
	//	switch (self->outdisp_dev) {
	//	case OUTPUT_HDMI:
	//		ioctl(self->fb_fd0, TCC_LCDC_HDMI_DISPLAY, &self->outdisp_info);
	//		printf("Output display: HDMI\n");
	//		break;
	//	case OUTPUT_COMPOSITE:
	//		ioctl(self->composite_fd, TCC_COMPOSITE_IOCTL_UPDATE, &self->outdisp_info);
	//		printf("Output display: Composite\n");
	//		break;
	//	case OUTPUT_COMPONENT:
	//		printf("Not support component output\n");
	//		break;
	//	}
	//}
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

		if (self->outdisp_dev) {
			self->outdisp_info.addr0 = base[0];
			self->outdisp_info.addr1 = base[1];
			self->outdisp_info.addr2 = base[2];
			self->outdisp_info.Frame_width	= self->vid_fmt.fmt.pix.width;
			self->outdisp_info.Frame_height	= self->vid_fmt.fmt.pix.height;
			self->outdisp_info.Image_width	= self->vid_fmt.fmt.pix.width;
			self->outdisp_info.Image_height	= self->vid_fmt.fmt.pix.height;

			switch (self->outdisp_dev) {
			case OUTPUT_HDMI:
				#if 1
				ioctl(self->fb_fd0, TCC_LCDC_HDMI_DISPLAY, &self->outdisp_info);
				#else
				self->outdisp_info.Lcdc_layer = LCDC_LAYER_2;
				self->outdisp_info.offset_x = 0;
				self->outdisp_info.offset_y = 0;
				self->outdisp_info.crop_right = 960;
				//self->outdisp_info.enable = 0;
				ioctl(self->fb_fd0, TCC_LCDC_HDMI_DISPLAY, &self->outdisp_info);

				self->outdisp_info.Lcdc_layer = LCDC_LAYER_1;
				self->outdisp_info.offset_x = 960;
				self->outdisp_info.offset_y = 0;
				self->outdisp_info.crop_right = 960;
				//self->outdisp_info.enable = 1;
				//self->outdisp_info.addr1 = base[2];
				//self->outdisp_info.addr2 = base[1];
				ioctl(self->fb_fd0, TCC_LCDC_HDMI_DISPLAY, &self->outdisp_info);
				#endif
				break;
			case OUTPUT_COMPOSITE:
				ioctl(self->composite_fd, TCC_COMPOSITE_IOCTL_UPDATE, &self->outdisp_info);
				break;
			case OUTPUT_COMPONENT:
				printf("Not support component output\n");
				break;
			}
		}
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

		if (self->use_scaler) {
			self->vout_qbuf.m.planes->reserved[VID_WIDTH] = self->vout_external_sc_fmt.fmt.win.w.width;
			self->vout_qbuf.m.planes->reserved[VID_HEIGHT] = self->vout_external_sc_fmt.fmt.win.w.height;
			self->vout_qbuf.m.planes->reserved[VID_CROP_LEFT] = self->vout_external_sc_fmt.fmt.win.w.left;
			self->vout_qbuf.m.planes->reserved[VID_CROP_TOP] = self->vout_external_sc_fmt.fmt.win.w.top;
			self->vout_qbuf.m.planes->reserved[VID_CROP_WIDTH] = self->vout_external_sc_fmt.fmt.win.w.width;
			self->vout_qbuf.m.planes->reserved[VID_CROP_HEIGHT] = self->vout_external_sc_fmt.fmt.win.w.height;
		} else {
			self->vout_qbuf.m.planes->reserved[VID_WIDTH] = self->vout_fmt.fmt.pix.width;
			self->vout_qbuf.m.planes->reserved[VID_HEIGHT] = self->vout_fmt.fmt.pix.height;
			self->vout_qbuf.m.planes->reserved[VID_CROP_LEFT] = 0;
			self->vout_qbuf.m.planes->reserved[VID_CROP_TOP] = 0;
			self->vout_qbuf.m.planes->reserved[VID_CROP_WIDTH] = self->vout_qbuf.m.planes->reserved[VID_WIDTH];
			self->vout_qbuf.m.planes->reserved[VID_CROP_HEIGHT] = self->vout_qbuf.m.planes->reserved[VID_HEIGHT];
		}

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


pmap_t pmap_sc;
static int pmap_sc_init = 0;

/*===========================================================================
FUNCTION
===========================================================================*/
void rsc_directly_draw_lcd(CameraDevice* self, struct v4l2_buffer *pBuf)
{
	unsigned int sc_src_w, sc_src_h;
	unsigned int sc_dst_w, sc_dst_h;
	unsigned int sc_src_paddr;
	unsigned int sc_dst_paddr;
	unsigned int src_base[3] = {0, 0, 0};
	unsigned int dst_base[3] = {0, 0, 0};

	#define SC_DST_BUF_NUMS	2
	static unsigned int sc_dst_buf[SC_DST_BUF_NUMS];
	static int sc_dst_buf_idx = -1;

	#if 0
	{
		static int ttcnt = 0;
		if ((ttcnt++%10) != 0)
			return;
	}
	#endif

	if (self->use_scaler) {
		if (pmap_sc_init == 0) {
			//pmap_get_info("fb_scale", &pmap_sc);
			pmap_get_info("video", &pmap_sc);
			pmap_sc_init = 1;
		}

		sc_src_w = self->vid_fmt.fmt.pix.width/*self->output_width*/;
		sc_src_h = self->vid_fmt.fmt.pix.height/*self->output_height*/;
		sc_dst_w = self->preview_width;
		sc_dst_h = self->preview_height;

		sc_dst_buf[0] = pmap_sc.base;
		sc_dst_buf[1] = sc_dst_buf[0] + ((sc_dst_w*sc_dst_h*2)/1024 +1)*1024;

		if ((sc_dst_buf_idx%(SC_DST_BUF_NUMS)) == 0)
			sc_dst_buf_idx = 0;
		
		if (pBuf->m.offset < self->pmap_camera.base)
			sc_src_paddr = pBuf->m.offset + self->pmap_camera.base;
		else
			sc_src_paddr = pBuf->m.offset;

		///sc_dst_paddr = pmap_sc.base;
		//sc_dst_paddr = sc_dst_buf[0];
		sc_dst_paddr = sc_dst_buf[sc_dst_buf_idx++];

		src_base[0] = sc_src_paddr;
		_get_plane_addrs(self->preview_fmt, sc_src_w, sc_src_h, &src_base[0], &src_base[1], &src_base[2]);

		dst_base[0] = sc_dst_paddr;
		_get_plane_addrs(self->preview_fmt, sc_dst_w, sc_dst_h, &dst_base[0], &dst_base[1], &dst_base[2]);

		tcc_scaler_yuv420_full(sc_src_w, sc_src_h, sc_src_w, sc_src_h, 
							   sc_dst_w, sc_dst_h, sc_dst_w, sc_dst_h, 
							   src_base[0], src_base[1], src_base[2], 
							   dst_base[0], dst_base[1], dst_base[2]);

		if (self->use_vout == 0) {
			self->overlay_config.sx = 0;
			self->overlay_config.sy = 0;
			self->overlay_config.width = sc_dst_w;
			self->overlay_config.height = sc_dst_h;
			self->overlay_config.format = vioc2fourcc(self->preview_fmt);
		} else {
			int ret;
			self->vout_external_sc_fmt.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
			self->vout_external_sc_fmt.fmt.win.w.left = 0;
			self->vout_external_sc_fmt.fmt.win.w.top = 0;
			self->vout_external_sc_fmt.fmt.win.w.width = sc_dst_w;
			self->vout_external_sc_fmt.fmt.win.w.height = sc_dst_h;
			ret = ioctl(self->vout_fd, VIDIOC_S_FMT, &self->vout_fmt);
			if (ret) {
				printf("error: VIDIOC_S_FMT(V4L2_BUF_TYPE_VIDEO_OVERLAY)\n");
			} else {
				printf("VIDIOC_S_FMT(V4L2_BUF_TYPE_VIDEO_OVERLAY)\n");
				printf("    left   = %d\n", self->vout_external_sc_fmt.fmt.win.w.left);
				printf("    top    = %d\n", self->vout_external_sc_fmt.fmt.win.w.top);
				printf("    width  = %d\n", self->vout_external_sc_fmt.fmt.win.w.width);
				printf("    height = %d\n", self->vout_external_sc_fmt.fmt.win.w.height);
			}
		}
	}

	/* update overlay configuration.
	 * because we can display different size output.
	 */
	if (self->use_vout == 0) {
		ioctl(self->overlay_fd, OVERLAY_SET_CONFIGURE, &self->overlay_config);
	}

	/* display
	 */
	if (self->use_scaler) {
		rsc_v4l2_qbuf(self, sc_dst_paddr);
	} else {
		if (pBuf->m.offset < self->pmap_camera.base)
			rsc_v4l2_qbuf(self, pBuf->m.offset + self->pmap_camera.base);
		else
			rsc_v4l2_qbuf(self, pBuf->m.offset);
	}
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

