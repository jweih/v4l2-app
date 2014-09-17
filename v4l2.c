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

#include <errno.h>
#include <time.h>
#include "main.h"

#include <linux/videodev2.h>
#include <mach/tcc_cam_ioctrl.h>

int qbuf_idx;

/**************************************************************************/
/*                                                                        */
/*                         Camera FuncTion			                       */
/*                                                                        */
/**************************************************************************/
static void record_file(CameraDevice *self, struct v4l2_buffer *buf)
{
	unsigned int width, height, tsize;
	unsigned char *base[3];
	unsigned int size[3];

	width = self->overlay_config.width;
	height = self->overlay_config.height;
	tsize = width * height;

	switch (self->preview_fmt) {
	case TCC_LCDC_IMG_FMT_YUV420SP:
		base[0] = self->buffers[buf->index];
		base[1] = base[0] + tsize;
		base[2] = base[1] + (tsize / 4);
		size[0] = tsize;
		size[1] = tsize / 4;
		size[2] = size[1];
		break;
	case TCC_LCDC_IMG_FMT_YUV422SP:
		base[0] = self->buffers[buf->index];
		base[1] = base[0] + tsize;
		base[2] = base[1] + (tsize / 2);
		size[0] = tsize;
		size[1] = tsize / 2;
		size[2] = size[1];
		break;
	case TCC_LCDC_IMG_FMT_YUV420ITL0:
	case TCC_LCDC_IMG_FMT_YUV420ITL1:
		base[0] = self->buffers[buf->index];
		base[1] = base[0] + tsize;
		base[2] = NULL;
		size[0] = tsize;
		size[1] = size[0] / 2;
		size[2] = 0;
		break;
	case TCC_LCDC_IMG_FMT_YUV422ITL0:
	case TCC_LCDC_IMG_FMT_YUV422ITL1:
		base[0] = self->buffers[buf->index];
		base[1] = base[0] + (width * height);
		base[2] = NULL;
		size[0] = tsize;
		size[1] = size[0];
		size[2] = 0;
	    break;
	case TCC_LCDC_IMG_FMT_UYVY:
	case TCC_LCDC_IMG_FMT_VYUY:
	case TCC_LCDC_IMG_FMT_YUYV:
	case TCC_LCDC_IMG_FMT_YVYU:
		base[0] = self->buffers[buf->index];
		base[1] = NULL;
		base[2] = NULL;
		size[0] = tsize * 2;
		size[1] = 0;
		size[2] = 0;
		break;
	}

	fwrite(base[0], 1, size[0], self->fp);
	if (base[1] != NULL)
		fwrite(base[1], 1, size[1], self->fp);
	if (base[2] != NULL)
		fwrite(base[2], 1, size[2], self->fp);
}
/*===========================================================================
FUNCTION
===========================================================================*/
static int _camif_init_format (CameraDevice *self, int width, int height)
{
    int result;
	unsigned int sample_size, i;
	
    memset(&self->vid_fmt, 0,  sizeof(struct v4l2_format));
    self->vid_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if((result = ioctl(self->fd, VIDIOC_G_FMT, &self->vid_fmt)) < 0)
	{
		DBug_printf(" ERROR :: cam ioctl() in function VIDIOC_G_FMT failed!\n");
	}
	else
	{
		printf("VIDIOC_G_FMT: get self[%d]->vid_fmt.fmt.pix info\n", self->camdev);
		printf("    .width  = %d\n", self->vid_fmt.fmt.pix.width);
		printf("    .height = %d\n", self->vid_fmt.fmt.pix.height);
		/* For do not use scaler 
		 *  - if width/height is same self->vid_fmt.fmt.pix.width/height
		 *    camera driver do not use scaler.
		 *  - rsc.c::rsc_init_lcd()
		 *    width: self->preview_width  = fb_info.xres;
		 *    heght: self->preview_height = fb_info.yres;
		 */
		width = self->vid_fmt.fmt.pix.width;
		height = self->vid_fmt.fmt.pix.height;

		switch (self->preview_fmt) {
		case TCC_LCDC_IMG_FMT_YUV420SP:
		case TCC_LCDC_IMG_FMT_YUV420ITL0:
		case TCC_LCDC_IMG_FMT_YUV420ITL1:
			self->vid_fmt.type					= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		    self->vid_fmt.fmt.pix.width			= width;
		    self->vid_fmt.fmt.pix.height		= height;
		    self->vid_fmt.fmt.pix.field			= V4L2_FIELD_INTERLACED;
		    self->vid_fmt.fmt.pix.sizeimage		= (width * height * 3) / 2;
			self->vid_fmt.fmt.pix.pixelformat	= vioc2fourcc(self->preview_fmt);
			break;
		case TCC_LCDC_IMG_FMT_YUV422SP:
		case TCC_LCDC_IMG_FMT_YUV422ITL0:
		case TCC_LCDC_IMG_FMT_YUV422ITL1:
		case TCC_LCDC_IMG_FMT_UYVY:
		case TCC_LCDC_IMG_FMT_VYUY:
		case TCC_LCDC_IMG_FMT_YUYV:
		case TCC_LCDC_IMG_FMT_YVYU:
		    self->vid_fmt.type					= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		    self->vid_fmt.fmt.pix.width			= width;
		    self->vid_fmt.fmt.pix.height		= height;
		    self->vid_fmt.fmt.pix.field			= V4L2_FIELD_INTERLACED;
		    self->vid_fmt.fmt.pix.sizeimage 	= width * height * 2;
			self->vid_fmt.fmt.pix.pixelformat	= vioc2fourcc(self->preview_fmt);
			break;
		}

	    if((result = ioctl(self->fd, VIDIOC_S_FMT, &self->vid_fmt)) < 0)
		{
			DBug_printf(" ERROR :: cam ioctl() in function VIDIOC_S_FMT failed!\n");
	    }
		else
		{
			printf("VIDIOC_S_FMT: get self[%d]->vid_fmt.fmt.pix info\n", self->camdev);
			printf("    .width  = %d\n", self->vid_fmt.fmt.pix.width);
			printf("    .height = %d\n", self->vid_fmt.fmt.pix.height);
		}

#if !defined(_PUSH_VSYNC_)
		self->overlay_config.sx = 0;
		self->overlay_config.sy = 0;
		self->overlay_config.width = width;
		self->overlay_config.height = height;
		self->overlay_config.format = self->vid_fmt.fmt.pix.pixelformat;
		ioctl(self->overlay_fd, OVERLAY_SET_CONFIGURE, &self->overlay_config);
		DBug_printf("[%s] OVERLAY_SET_CONFIGURE: (%d x %d) fmt(0x%x)\n", __func__, 
				self->overlay_config.width, self->overlay_config.height,
				self->overlay_config.format);
#endif

		/* 
		 * Setup VIQE histogram 
		 */
		self->viqe_type.inpath = 0;	//0: RDMA14, 1:VIN3(not used)
		self->viqe_type.width = self->vid_fmt.fmt.pix.width;
		self->viqe_type.height = self->vid_fmt.fmt.pix.height;
		self->viqe_type.hoff = 10;
		self->viqe_type.voff = 10;

		self->viqe_type.xstart = 0;
		self->viqe_type.xend = self->viqe_type.width;
		self->viqe_type.ystart = 0;
		self->viqe_type.yend = self->viqe_type.height;
		
		if ((self->viqe_type.height % self->viqe_type.hoff) == 0) {
			sample_size = (self->viqe_type.width / self->viqe_type.hoff) * (self->viqe_type.height / self->viqe_type.voff);
		} else {
			sample_size = (self->viqe_type.width / self->viqe_type.hoff) * ((self->viqe_type.height / self->viqe_type.voff) + 1);
		}
		self->viqe_type.samplesize = sample_size;

		for (i = 1; i < 16; i++) {
			self->viqe_type.seg[i - 1] = 0x10 * i;	// segment unit: 0x10
		}
		self->viqe_type.seg[15] = 0;

		for (i = 0; i < 16; i++) {
			self->viqe_type.scale[i] = 0xFF;
		}

		self->viqe_type.stDMA.width = self->viqe_type.width;
		self->viqe_type.stDMA.height = self->viqe_type.height;
		#ifdef USE_PIX_FMT_YUV420
		self->viqe_type.stDMA.imgfmt = TCC_LCDC_IMG_FMT_YUV420SP;	// YCbCr 4:2:0 Separated format
		#else
		self->viqe_type.stDMA.imgfmt = TCC_LCDC_IMG_FMT_YUV422SP;	// YCbCr 4:2:2 Separated format
		#endif

	}
    return result;
}

/*===========================================================================
FUNCTION
===========================================================================*/
static int _camif_init_buffer(CameraDevice *self)
{
    int result;
    struct v4l2_buffer buf;
	
    memset(&self->vid_buf, 0,  sizeof(struct v4l2_requestbuffers));

	if(self->cam_mode == MODE_CAPTURE)
    	self->vid_buf.count  = 1;    
	else
    	self->vid_buf.count  = NUM_VIDBUF;  
	
    self->vid_buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    self->vid_buf.memory = V4L2_MEMORY_MMAP;

	// internal buffer allocation!!
    if((result = ioctl(self->fd, VIDIOC_REQBUFS, &self->vid_buf)) < 0)
	{
        DBug_printf(" ERROR :: cam ioctl() in function VIDIOC_REQBUFS failed!");
        exit (EXIT_FAILURE);
	}
	else
	{    
		if (self->vid_buf.count < 1) 
		{
	        DBug_printf("ERROR :: Insufficient buffer memory on camera\n");
	        exit (EXIT_FAILURE);
	    }
		DBug_printf(" Buffer Count : %d ..\n", self->vid_buf.count);
	}
	
    memset(&buf, 0,  sizeof(struct v4l2_buffer));
    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = 0;

	// get internal buffer and mmap!!
    for (buf.index=0; buf.index<self->vid_buf.count; buf.index++) 
	{
        if (ioctl (self->fd, VIDIOC_QUERYBUF, &buf) < 0) 
		{
	        DBug_printf(" ERROR :: cam ioctl() in function VIDIOC_QUERYBUF failed!");
		    return -1;
		}

		if (buf.m.offset < self->pmap_camera.base)
		{
			self->buffers[buf.index] = mmap(0, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, self->fd, buf.m.offset + self->pmap_camera.base);
		}
		else
		{
        	self->buffers[buf.index] = mmap(0, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, self->fd, buf.m.offset);
		}

		if (MAP_FAILED == self->buffers[buf.index]) 
		{
		    DBug_printf("mmap failed\n");
		    return -1;
		}
    }

	qbuf_idx = -1;
	
    return result;
}

/*===========================================================================
FUNCTION
===========================================================================*/
static void _camif_uninit_buffer(CameraDevice *self)
{
	struct v4l2_buffer buf;
	int i;

	memset(&buf, 0,  sizeof(struct v4l2_buffer));

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	
	for (i=0; i<self->vid_buf.count; i++) 
	{
	    if ( ioctl(self->fd, VIDIOC_QUERYBUF, &buf) < 0) 
		{
			break;
	    }
	    munmap(self->buffers[buf.index], buf.length);
	}

}


/*===========================================================================
FUNCTION
===========================================================================*/
void open_device(CameraDevice* self)
{
	// 2. Open LCD Device !!
	if((self->fb_fd0 = open(FB_DEVICE_NAME, O_RDWR)) < 0)
	{
		DBug_printf("ERROR ::  LCD Driver Open : fd[%d]\n", self->fb_fd0);
		exit(1);
	}

#if !defined(_PUSH_VSYNC_)
	if((self->overlay_fd = open(OVERLAY_DEVICE_NAME, O_RDWR)) < 0)
	{
		DBug_printf("ERROR ::  Overlay Driver Open : fd[%d]\n", self->overlay_fd);
		exit(1);
	}
#endif

	// open composite
	if (self->outdisp_dev == OUTPUT_COMPOSITE) {
		if ((self->composite_fd = open(COMPOSITE_DEVICE, O_RDWR)) < 0) {
			DBug_printf("ERROR ::  Composite Driver Open : fd[%d]\n", self->composite_fd);
			exit(1);
		}
	}
	
	// open viqe
	if ((self->viqe_fd = open(VIQE_DEVICE_NAME, O_RDWR)) < 0) {
		DBug_printf("ERROR ::  VIQE Driver Open : fd[%d]\n", self->overlay_fd);
		//exit(1);
	}

	if (self->record)
		self->fp = fopen("./rec_data", "wb");

	if (self->i2c_slave_addr) {
		if ((self->i2c_fd = open(self->i2c_dev_port, O_RDWR)) < 0) {
			printf("i2c: %s driver open failed\n", self->i2c_dev_port);
			exit(1);
		}
	}

	return;
}

/*===========================================================================
FUNCTION
===========================================================================*/
void close_device(CameraDevice* self)
{
	camif_stop_stream(self);
	_camif_uninit_buffer(self);

	// 1. Close Camera Device!!
    close(self->fd);

	// 2. Close LCD Device!!	
    close(self->fb_fd0);

#if !defined(_PUSH_VSYNC_)
	close(self->overlay_fd);
#endif

	if (self->outdisp_dev == OUTPUT_COMPOSITE)
		close(self->composite_fd);

	// close viqe device
	close(self->viqe_fd);

	if (self->record)
		fclose(self->fp);
}

/*===========================================================================
FUNCTION
===========================================================================*/
void  init_camera_data(CameraDevice *self, unsigned int preview_fmt, int opt)
{
    //memset (self, 0, sizeof (CameraDevice));
	
    self->fd 						= -1;
    self->preview_width				= PREVIEW_WIDTH;
    self->preview_height			= PREVIEW_HEIGHT;
	self->cam_mode					= MODE_START;
	self->preview_fmt				= preview_fmt;

	if (opt == 1)
		self->timestamp = 1;
	else if (opt == 2)
		self->record = 1;
}

/*===========================================================================
FUNCTION
===========================================================================*/
int camif_get_dev_info (CameraDevice *self)
{
    int result;
	
    if((result = ioctl (self->fd, VIDIOC_QUERYCAP, &self->vid_cap)) < 0)
	{
		DBug_printf(" ERROR :: cam ioctl() in _init_device_info failed");
    }
    else 
	{
		DBug_printf("Driver: %s\n", self->vid_cap.driver);
		DBug_printf("Card: %s\n", self->vid_cap.card);
		DBug_printf("Capabilities: 0x%x\n", (unsigned int)(self->vid_cap.capabilities));
    }
	
    return result;
}

/*===========================================================================
FUNCTION
===========================================================================*/
//static bool switch_flag = 1;
//static unsigned int switch_count = 0;
int camif_get_frame(CameraDevice *self)
{
    int res;
	struct v4l2_buffer buf;
	int buf_idx;
	
	memset(&buf, 0, sizeof(struct v4l2_buffer));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if ((res = ioctl (self->fd, VIDIOC_DQBUF, &buf)) < 0) {
		switch(errno) {
		case EAGAIN:
			DBug_printf("[%s] VIDIOC_DQBUF: errno #%d (EAGAIN)\n", self->dev_name, errno);
			return 0;
		case EIO:
			default:
			DBug_printf("[%s] VIDIOC_DQBUF: errno #%d\n", self->dev_name, errno);
			return -1;
		}
	}

	buf_idx = buf.index;

	if (self->record)
		record_file(self, &buf);

	if (self->timestamp)
		rsc_buf_timestamp_logprint(&buf);

	if (1)
	{
		if (self->display) {
			/*
			 * VIQE histogram
			 */
			if (self->histogram_excute) {
				if (buf.m.offset < self->pmap_camera.base) {
					self->viqe_type.stDMA.base0 = buf.m.offset + self->pmap_camera.base;
				} else {
					self->viqe_type.stDMA.base0 = buf.m.offset;
				}

				#ifdef USE_PIX_FMT_YUV420
				self->viqe_type.stDMA.base1 = self->viqe_type.stDMA.base0 + (self->preview_width*self->preview_height);
				self->viqe_type.stDMA.base2 = self->viqe_type.stDMA.base1 + (self->preview_width*self->preview_height/4);
				//{/* test change black, white */
				//	unsigned int size = (self->preview_width*self->preview_height);
				//	if (switch_flag)
				//		memset(self->buffers[buf.index], 0xf0, size);	//white
				//	else
				//		memset(self->buffers[buf.index], 0x10, size);	//black
				//	memset(self->buffers[buf.index] + size, 0x80, (size / 4));
				//	memset(self->buffers[buf.index] + size + (size / 4), 0x80, (size/4));
				//	if (switch_count++ == 50) {
				//		switch_flag = !switch_flag;
				//		switch_count = 0;
				//	}
				//}
				#else
				self->viqe_type.stDMA.base1 = self->viqe_type.stDMA.base0 + (self->preview_width*self->preview_height);
				self->viqe_type.stDMA.base2 = self->viqe_type.stDMA.base1 + (self->preview_width*self->preview_height/2);
				#endif

				if (ioctl(self->viqe_fd, IOCTL_VIQE_HISTOGRAM_INIT, &self->viqe_type) < 0) {
					DBug_printf("[cam0] error: IOCTL_VIQE_HISTOGRAM_INIT\n");
				} else {
					if (ioctl(self->viqe_fd, IOCTL_VIQE_HISTOGRAM_EXECUTE, self->histogram) < 0) {
						DBug_printf("[cam0] error: IOCTL_VIQE_HISTOGRAM_EXECUTE\n");
					} else {
						//DBug_printf("[%c] ", switch_flag?'W':'B');
						DBug_printf("HISTOGRMA DATA[16] = {%4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d, %4d}\n", 
							self->histogram[0], self->histogram[1], self->histogram[2], self->histogram[3],
							self->histogram[4], self->histogram[5], self->histogram[6], self->histogram[7],
							self->histogram[8], self->histogram[9], self->histogram[10], self->histogram[11],
							self->histogram[12], self->histogram[13], self->histogram[14], self->histogram[15]);

						ioctl(self->viqe_fd, IOCTL_VIQE_HISTOGRAM_DEINIT, NULL);
					}
				}

				self->histogram_excute = 0;
			}

			/*
			 * display video
			 */
			rsc_directly_draw_lcd(self, &buf);
		}
	}
	else
	{
		rsc_process_image(self, (uint8_t*)(buf.m.offset));
		rsc_draw_lcd(self);
	}

//	if (qbuf_idx != -1)
	{
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
//		buf.index = qbuf_idx;
		buf.index = buf_idx;
	
		if (ioctl(self->fd, VIDIOC_QBUF, &buf) < 0) {
			DBug_printf("%s, VIDIOC_QBUF failed\n", __func__);
			return -1;
		}
	}

	qbuf_idx = buf_idx;
	
    return 1;
}


/*===========================================================================
FUNCTION
===========================================================================*/
void camif_set_queryctrl(CameraDevice *self, unsigned int ctrl_id, int value)
{
    struct v4l2_queryctrl q;
    struct v4l2_control c;

	/*
	 * VIDIOC_QUERYCTRL
	 * Query the attibutes of a (camera) control
	 */
    memset(&q, 0, sizeof(struct v4l2_queryctrl));
    q.id = ctrl_id;

    if (ioctl(self->fd, VIDIOC_QUERYCTRL, &q) < 0 ) {
		if (errno != EINVAL)
			perror("VIDIOC_QUERYCTRL");
		else
			printf("V4L2_CID (0x%08X) is not supported\n", q.id);
		goto exit;
    } else if (q.flags & V4L2_CTRL_FLAG_DISABLED) {
    	printf("V4L2_CID (0x%08X) is supported. But disabled\n", q.id);
		goto exit;
    }

	printf("[VIDIOC_QUERYCTRL]\n"
		   "v4l2_queryctrl.id            = 0x%08X\n"
		   "              .type          = %d\n"
		   "              .name          = %s\n"
		   "              .minimum       = %d\n"
		   "              .maximum       = %d\n"
		   "              .step          = %d\n"
		   "              .default_value = %d\n"
		   "              .flags         = 0x%08X\n"
		   "              .reserved[2]   = %d, %d\n"
		   , q.id, q.type, q.name
		   , q.minimum, q.maximum, q.step, q.default_value
		   , q.flags, q.reserved[0], q.reserved[1]);

	/*
	 * VIDIOC_G_CTRL
	 * Get the value of a control
	 */
	memset(&c, 0, sizeof(struct v4l2_control));
    c.id = ctrl_id;

	if (ioctl(self->fd, VIDIOC_G_CTRL, &c) < 0) {
		if (errno != EINVAL)
			perror("VIDIOC_G_CTRL");
		else
			printf("V4L2_CID (0x%08X) is not supported\n", q.id);
		goto exit;
	} else {
		printf("[VIDIOC_G_CTRL] current value is %d\n", c.value);
	}

	/*
	 * VIDIOC_S_CTRL
	 * Set the value of a control
	 */
    memset(&c, 0, sizeof(struct v4l2_control));
    c.id = ctrl_id;

    if (value < q.minimum || value > q.maximum) {
		c.value = q.default_value;
		printf("[VIDIOC_S_CTRL] your value(%d) out of range(%d ~ %d)\n"
			   "                so, set default_value(%d)\n"
			   , value, q.minimum, q.maximum, q.default_value);
	} else {
		c.value = value;
	}

    if (ioctl(self->fd, VIDIOC_S_CTRL, &c) < 0) {
		if (errno != EINVAL)
			perror("VIDIOC_S_CTRL");
		else
			printf("V4L2_CID (0x%08X) is not supported\n", q.id);
		goto exit;
    } else {
    	printf("[VIDIOC_S_CTRL] success set value(%d)\n", c.value);
    }

exit:
	return;
}

/*===========================================================================
FUNCTION
===========================================================================*/
void camif_set_resolution (CameraDevice *self, int width, int height)
{
	_camif_uninit_buffer(self);

    if (_camif_init_format(self, width, height) < 0) 
	{
    	exit(-1);
    }

    if (_camif_init_buffer(self) < 0) 
	{
		exit(-1);
    }	
}

/*===========================================================================
FUNCTION
===========================================================================*/
void camif_set_overlay(CameraDevice *self, int buffer_value)
{

}

/*===========================================================================
FUNCTION
===========================================================================*/
void camif_start_stream(CameraDevice *self)
{
    struct v4l2_buffer buf;
    int type;

	if(self->cam_mode == MODE_PREVIEW)
		return;

	for (buf.index=0; buf.index<self->vid_buf.count; buf.index++) 
	{
	    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	    buf.memory      = V4L2_MEMORY_MMAP;

	    if (ioctl (self->fd, VIDIOC_QBUF, &buf) < 0) 
		{
			DBug_printf("VIDIOC_QBUF\n");
			exit(EXIT_FAILURE);
	    }
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl (self->fd, VIDIOC_STREAMON, &type) < 0) 
	{
		DBug_printf("ERROR :: Can't VIDIOC_STREAMON\n");
		exit(EXIT_FAILURE);
	}

	self->cam_mode = MODE_PREVIEW;

#if defined(_PUSH_VSYNC_)
	ioctl(self->fb_fd0, TCC_LCDC_VIDEO_START_VSYNC, 5);//VOUT_VPU_BUFF_COUNT);
#endif
}

/*===========================================================================
FUNCTION
===========================================================================*/
void camif_stop_stream(CameraDevice *self)
{
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if(self->cam_mode != MODE_PREVIEW)
		return;
	
	if (ioctl (self->fd, VIDIOC_STREAMOFF, &type) < 0) 
	{
		DBug_printf("ERROR :: Can't VIDIOC_STREAMOFF\n");
		exit(EXIT_FAILURE);
	}

	self->cam_mode = MODE_PREVIEW_STOP;

#if defined(_PUSH_VSYNC_)
	ioctl(self->fb_fd0, TCC_LCDC_VIDEO_END_VSYNC, 0);
#endif
}

/*===========================================================================
FUNCTION
===========================================================================*/
void  camif_capture(CameraDevice *self)
{

}

