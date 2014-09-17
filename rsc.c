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


/*===========================================================================
FUNCTION
===========================================================================*/
void rsc_init_lcd(CameraDevice* self)
{
    static struct fb_fix_screeninfo fb_fix;
    static struct fb_var_screeninfo fb_info;

	typedef struct _ImageInfoType {
		unsigned int ImageFmt;
		unsigned int pos_x;
		unsigned int pos_y;
		unsigned int width;
		unsigned int height;
	} ImageInfoType;

	ImageInfoType ImgInfo;

    ioctl(self->fb_fd0, FBIOGET_FSCREENINFO, &fb_fix);
    ioctl(self->fb_fd0, FBIOGET_VSCREENINFO, &fb_info);

#if defined(_PUSH_VSYNC_TT_TEST_00_)
	self->preview_width  = PREVIEW_WIDTH;
	self->preview_height = PREVIEW_HEIGHT;
	self->fb_xres = fb_info.xres;
	self->fb_yres = fb_info.yres;
	DBug_printf("LCD mode: %d, Preview: %d x %d <tt>\n", self->rt_mode, self->preview_width, self->preview_height);
#else
	self->preview_width  = fb_info.xres;
	self->preview_height = fb_info.yres;
	DBug_printf("LCD mode: %d, Preview: %d x %d\n", self->rt_mode, self->preview_width, self->preview_height);

	ImgInfo.width 	= self->preview_width;
	ImgInfo.height 	= self->preview_height;
	ImgInfo.pos_x 	= (fb_info.xres > ImgInfo.width)  ? (fb_info.xres-ImgInfo.width)/2  : 0;
	ImgInfo.pos_y 	= (fb_info.yres > ImgInfo.height) ? (fb_info.yres-ImgInfo.height)/2 : 0;

	if (self->outdisp_dev) {
		self->outdisp_info.enable		= 1;
		self->outdisp_info.Lcdc_layer	= LCDC_LAYER_2;
		self->outdisp_info.fmt			= self->preview_fmt;

		self->outdisp_info.offset_x		= fb_info.xoffset;
		self->outdisp_info.offset_y		= fb_info.yoffset;

		self->outdisp_info.Frame_width	= self->output_width/*self->preview_width*/;
		self->outdisp_info.Frame_height	= self->output_height/*self->preview_height*/;

		self->outdisp_info.crop_left	= 0;
		self->outdisp_info.crop_top		= 0;
		self->outdisp_info.crop_right	= self->outdisp_info.Frame_width;
		self->outdisp_info.crop_bottom	= self->outdisp_info.Frame_height;

		self->outdisp_info.Image_width	= self->output_width/*self->preview_width*/;
		self->outdisp_info.Image_height	= self->output_height/*self->preview_height*/;
	}
#endif

#if !defined(_PUSH_VSYNC_)
	self->overlay_config.sx = ImgInfo.pos_x;
	self->overlay_config.sy = ImgInfo.pos_y;
	self->overlay_config.width = ImgInfo.width;
	self->overlay_config.height = ImgInfo.height;
	//self->overlay_config.format = self->vid_fmt.fmt.pix.pixelformat;
	self->overlay_config.format = vioc2fourcc(self->preview_fmt);

	ioctl(self->overlay_fd, OVERLAY_SET_CONFIGURE,&self->overlay_config);
	DBug_printf("[%s] OVERLAY_SET_CONFIGURE: (%d x %d) fmt(0x%x)\n", __func__, 
				self->overlay_config.width, self->overlay_config.height,
				self->overlay_config.format);
#endif

	if (self->outdisp_dev) {
		switch (self->outdisp_dev) {
		case OUTPUT_HDMI:
			ioctl(self->fb_fd0, TCC_LCDC_HDMI_DISPLAY, &self->outdisp_info);
			printf("Output display: HDMI\n");
			break;
		case OUTPUT_COMPOSITE:
			ioctl(self->composite_fd, TCC_COMPOSITE_IOCTL_UPDATE, &self->outdisp_info);
			printf("Output display: Composite\n");
			break;
		case OUTPUT_COMPONENT:
			printf("Not support component output\n");
			break;
		}
	}
}

/*===========================================================================
FUNCTION
===========================================================================*/
void rsc_set_lcd_ch0(CameraDevice* self, unsigned char en)
{
#if !defined(_PUSH_VSYNC_)
	if(en)
	{
		if(self->fb_fd0 >= 0)
			ioctl(self->fb_fd0,TCC_LCDC_SET_DISABLE, NULL);
	}
	else
	{
		if(self->overlay_fd >= 0)
			ioctl(self->overlay_fd, OVERLAY_SET_DISABLE,NULL);

		if(self->fb_fd0 >= 0)
			ioctl(self->fb_fd0,TCC_LCDC_SET_ENABLE, NULL);
	}
#endif
}

/*===========================================================================
FUNCTION
===========================================================================*/
void rsc_set_lcd_addr(CameraDevice* self, unsigned int addr)
{
	unsigned int base[3] = {0, 0, 0};
	unsigned int width, height;

	//width = self->preview_width;
	//height = self->preview_height;
	width = self->overlay_config.width;
	height = self->overlay_config.height;

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
	}

	if(debug_flg)
		printf("[APP] addr = %x\n", addr);

#if !defined(_PUSH_VSYNC_)
	ioctl(self->overlay_fd, OVERLAY_QUEUE_BUFFER,base);
#endif

	if (self->outdisp_dev) {
		self->outdisp_info.addr0 = base[0];
		self->outdisp_info.addr1 = base[1];
		self->outdisp_info.addr2 = base[2];

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

/*===========================================================================
FUNCTION: rsc_video_push_vsync
===========================================================================*/
void rsc_video_push_vsync(CameraDevice* self, struct v4l2_buffer *pBuf)
{
	static unsigned int unique_id = 0;
	long i_time_stamp=0;
	signed long i_sync_time=0;
	signed long i_buffer_id = unique_id++;
//	signed long i_buffer_id = pBuf->index;
//	signed long i_interlace_flag=0;
//	unsigned long i_decID=0;

//	unsigned long PA_Y;
//	unsigned long PA_U;
//	unsigned long PA_V;
//	unsigned char *input_src_ptr;
	unsigned int base[3] = {0, 0, 0};

	struct tcc_lcdc_image_update_extend lcdc_display;

//	input_src_ptr = (p_buffer->pBuffer);

//	PA_Y = (*(input_src_ptr+3)<<24) | (*(input_src_ptr+2)<<16) | (*(input_src_ptr+1)<<8) | *(input_src_ptr) ; input_src_ptr+=4;
//	PA_U = (*(input_src_ptr+3)<<24) | (*(input_src_ptr+2)<<16) | (*(input_src_ptr+1)<<8) | *(input_src_ptr) ; input_src_ptr+=4;
//	PA_V = (*(input_src_ptr+3)<<24) | (*(input_src_ptr+2)<<16) | (*(input_src_ptr+1)<<8) | *(input_src_ptr) ; input_src_ptr+=4;

	base[0] = (unsigned char *)(pBuf->m.offset);
	base[1] = base[0] + (self->preview_width*self->preview_height);
	base[2] = base[1] + (self->preview_width*self->preview_height/4);

	i_sync_time 	= (signed long) (imdate() / 1000);	//(itv_clock_ConverSystem(i_decID, imdate()) / 1000);
	i_time_stamp	= (signed long) (((mtime_t)pBuf->timestamp.tv_sec * 1000000 + (mtime_t)pBuf->timestamp.tv_usec) / 1000);	//(itv_clock_ConverSystem(i_decID, p_buffer->nTimeStamp) / 1000);
	
	lcdc_display.Lcdc_layer = LCDC_LAYER_0;
	lcdc_display.enable 	= 1;
	lcdc_display.addr0		= base[0];	//PA_Y;
	lcdc_display.addr1		= base[1];	//PA_U;
	lcdc_display.addr2		= base[2];	//PA_V;

	lcdc_display.offset_x = 0;
	lcdc_display.offset_y = 0;

	lcdc_display.Frame_width	= self->preview_width;	//((p_priv->i_src_width + 15) >> 4) << 4;
	lcdc_display.Frame_height	= self->preview_height; //p_priv->i_src_height;
#if defined(_PUSH_VSYNC_TT_TEST_00_)
	lcdc_display.Image_width	= self->fb_xres;	//p_priv->i_lcd_width;
	lcdc_display.Image_height	= self->fb_yres;	//p_priv->i_lcd_height;
#else
	lcdc_display.Image_width	= self->preview_width;	//p_priv->i_lcd_width;
	lcdc_display.Image_height	= self->preview_height; //p_priv->i_lcd_height;
#endif
	lcdc_display.fmt = TCC_LCDC_IMG_FMT_YUV420SP;//TCC_LCDC_IMG_FMT_YUV420ITL0;
		
	lcdc_display.time_stamp 		= i_time_stamp;
	lcdc_display.sync_time			= i_sync_time;
	lcdc_display.buffer_unique_id	= i_buffer_id;
	lcdc_display.overlay_used_flag	= 1;
	lcdc_display.outputMode 		= OUTPUT_NONE;
		
	lcdc_display.deinterlace_mode = 1;//1;//(i_interlace_flag & 0x40000000) ? 1 : 0;
	lcdc_display.odd_first_flag = 1;//(i_interlace_flag & 0x10000000) ? 1 : 0;

	lcdc_display.first_frame_after_seek = 0;//(i_interlace_flag & 0x00000001) ? 1 : 0;
	lcdc_display.output_path			= 1;//(i_interlace_flag & 0x00000002) ? 1 : 0;

	ioctl(self->fb_fd0, TCC_LCDC_VIDEO_PUSH_VSYNC, &lcdc_display);
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
	unsigned int sc_src_fmt;
	unsigned int sc_dst_fmt;
	unsigned int base[3] = {0, 0, 0};
	unsigned int src_base[3] = {0, 0, 0};
	unsigned int dst_base[3] = {0, 0, 0};
	int scaler_fd;
	tcc_scaler_info_t src;
	tcc_scaler_info_t dst;

	#define SC_DST_BUF_NUMS	2
	static unsigned int sc_dst_buf[SC_DST_BUF_NUMS];
	static int sc_dst_buf_idx = -1;

#if defined(_PUSH_VSYNC_)
	rsc_video_push_vsync(self, pBuf);
#else

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

		self->overlay_config.sx = 0;
		self->overlay_config.sy = 0;
		self->overlay_config.width = sc_dst_w;
		self->overlay_config.height = sc_dst_h;
		self->overlay_config.format = vioc2fourcc(self->preview_fmt);
	}

	/* update overlay configuration.
	 * because we can display different size output.
	 */
	ioctl(self->overlay_fd, OVERLAY_SET_CONFIGURE, &self->overlay_config);

	/* display
	 */
	if (self->use_scaler) {
		rsc_set_lcd_addr(self, sc_dst_paddr);
	} else {
		if (pBuf->m.offset < self->pmap_camera.base)
			rsc_set_lcd_addr(self, pBuf->m.offset + self->pmap_camera.base);
		else
			rsc_set_lcd_addr(self, pBuf->m.offset);
	}
#endif
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

