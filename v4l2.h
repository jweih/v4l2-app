/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

               Camera    I n t e r f a c e    M O D U L E

                        EDIT HISTORY FOR MODULE

when        who       what, where, why
--------    ---       -------------------------------------------------------
10/xx/08   ZzaU      Created file.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/


#ifndef __CAM_V4L2_H__
#include "main.h"

#define __CAM_V4L2_H__

#define NUM_VIDBUF		8

#include "pmap.h"

#define fourcc2char(fourcc) \
        ((char) ((fourcc)     &0xff)), \
        ((char) (((fourcc)>>8 )&0xff)), \
        ((char) (((fourcc)>>16)&0xff)), \
        ((char) (((fourcc)>>24)&0xff))


// fb
#define TCC_LCDC_SET_ENABLE				0x0050
#define TCC_LCDC_SET_DISABLE			0x0051
//...

// overlay
#define OVERLAY_GET_POSITION		10
#define OVERLAY_GET_SCREEN_INFO		20
#define OVERLAY_SET_POSITION		30
#define OVERLAY_QUEUE_BUFFER		40
#define OVERLAY_SET_CONFIGURE		50
#define OVERLAY_SET_DISABLE			60
#define OVERLAY_SET_CROMA			70

#define OVERLAY_FORBID				100

//tcc_fb_ioctl_code.h
#define	TCC_LCDC_VIDEO_START_VSYNC		0x0060
#define	TCC_LCDC_VIDEO_END_VSYNC		0x0061
#define	TCC_LCDC_VIDEO_PUSH_VSYNC		0x0062
#define	TCC_LCDC_VIDEO_GET_DISPLAYED	0x0063
#define	TCC_LCDC_VIDEO_CLEAR_FRAME		0x0064

//typedef enum
//{
//	OUTPUT_NONE,
//	OUTPUT_HDMI,
//	OUTPUT_COMPOSITE,
//	OUTPUT_COMPONENT,
//	OUTPUT_MAX
//}OUTPUT_SELECT;
//
//typedef enum{
//	LCDC_LAYER_0, 
//	LCDC_LAYER_1, 
//	LCDC_LAYER_2, 
//	LCDC_LAYER_MAX
//}LCD_IMG_LAYER_TYPE;


//typedef enum{
//	TCC_LCDC_IMG_FMT_1BPP,
//	TCC_LCDC_IMG_FMT_2BPP,
//	TCC_LCDC_IMG_FMT_4BPP,
//	TCC_LCDC_IMG_FMT_8BPP,
//	TCC_LCDC_IMG_FMT_RGB332 = 8,
//	TCC_LCDC_IMG_FMT_RGB444 = 9,
//	TCC_LCDC_IMG_FMT_RGB565 = 10,
//	TCC_LCDC_IMG_FMT_RGB555 = 11,
//	TCC_LCDC_IMG_FMT_RGB888 = 12,
//	TCC_LCDC_IMG_FMT_RGB666 = 13,
//	TCC_LCDC_IMG_FMT_UYVY = 22,
//	TCC_LCDC_IMG_FMT_VYUY = 23,
//	TCC_LCDC_IMG_FMT_YUV420SP = 24,	
//	TCC_LCDC_IMG_FMT_YUV422SP = 25, 
//	TCC_LCDC_IMG_FMT_YUV422SQ = 26, 
//	TCC_LCDC_IMG_FMT_YUYV = 26,
//	TCC_LCDC_IMG_FMT_YVYU = 27,
//	TCC_LCDC_IMG_FMT_YUV420ITL0 = 28, 
//	TCC_LCDC_IMG_FMT_YUV420ITL1 = 29, 
//	TCC_LCDC_IMG_FMT_YUV422ITL0 = 30, 
//	TCC_LCDC_IMG_FMT_YUV422ITL1 = 31, 
//	TCC_LCDC_IMG_FMT_MAX
//}TCC_LCDC_IMG_FMT_TYPE;

//typedef struct  
//{
//	unsigned lcdc_num;
//	unsigned layer_num;
//} lcdc_layerctrl_params;

struct tcc_lcdc_image_update_extend
{
	unsigned int Lcdc_layer;
	unsigned int enable;
	unsigned int Frame_width;
	unsigned int Frame_height;

	unsigned int Image_width;
	unsigned int Image_height;
	unsigned int offset_x; //position
	unsigned int offset_y; 	//position
	unsigned int addr0;
	unsigned int addr1;
	unsigned int addr2;
	unsigned int fmt;	//TCC_LCDC_IMG_FMT_TYPE
	unsigned int on_the_fly; // 0: not use , 1 : scaler0 ,  2 :scaler1	
	int crop_top; 
	int crop_bottom;
	int crop_left;
	int crop_right;
	
//#if defined(CONFIG_TCC_VIDEO_DISPLAY_BY_VSYNC_INT) || defined(TCC_VIDEO_DISPLAY_BY_VSYNC_INT)
	int time_stamp;
	int sync_time;
	int first_frame_after_seek;
	unsigned int buffer_unique_id;
	unsigned int overlay_used_flag;
	OUTPUT_SELECT outputMode;
	int output_path;

	int deinterlace_mode;
	int odd_first_flag;
	int m2m_mode;
	int output_toMemory;
	int frameInfo_interlace;
//#endif

	int one_field_only_interlace;

	int MVCframeView;
	unsigned int MVC_Base_addr0;
	unsigned int MVC_Base_addr1;
	unsigned int MVC_Base_addr2;

	unsigned int dst_addr0;
	unsigned int dst_addr1;
	unsigned int dst_addr2;

	int max_buffer;
	int ex_output;

	unsigned int codec_id;
};

typedef enum{
	MODE_START = 0,
	MODE_PREVIEW,
	MODE_PREVIEW_STOP,
	MODE_CAPTURE
}camera_mode;


typedef struct
{
	unsigned short 			chromakey;
	
	unsigned char			mask_r;
	unsigned char			mask_g;
	unsigned char			mask_b;
	
	unsigned char			key_y;
	unsigned char			key_u;
	unsigned char			key_v;
	
}si_chromakey_info;

typedef struct
{
	unsigned short 			start_x;
	unsigned short 			start_y;
	unsigned short 			width;
	unsigned short 			height;
	
	unsigned int 			buff_offset;

	si_chromakey_info		chromakey_info;			
}cif_SuperImpose;

typedef struct
{
	uint32_t sx;
	uint32_t sy;
	uint32_t width;
	uint32_t height;
	uint32_t format;
	uint32_t transform;
} overlay_config_t;

/*======================== VIQE HISTOGRAM ==================================*/
#define IOCTL_VIQE_HISTOGRAM_INIT		0xBF0
#define IOCTL_VIQE_HISTOGRAM_DEINIT		0xBF1
#define IOCTL_VIQE_HISTOGRAM_SETPARAM	0xBF2
#define IOCTL_VIQE_HISTOGRAM_GETPARAM	0xBF3
#define IOCTL_VIQE_HISTOGRAM_EXECUTE	0xBF4
#define IOCTL_VIQE_HISTOGRAM_INPATH		0xBF5

typedef struct
{
    unsigned int    idx;
    unsigned int    width;
    unsigned int    height;
    unsigned int    base0;
	unsigned int    base1;
	unsigned int    base2;
    unsigned int    imgfmt;
} VIQE_DMAPARAM;

typedef struct
{
    unsigned int    inpath;
    unsigned int    width;      // VIQE_CTRL
    unsigned int    height;
    unsigned int    hoff;       // HIS_CONFIG
    unsigned int    voff;
    unsigned int    samplesize;
    unsigned int    xstart;     // Region
    unsigned int    xend;
    unsigned int    ystart;
    unsigned int    yend;
    unsigned char   seg[16];
    unsigned char   scale[16];
    unsigned char   lut[256];
    VIQE_DMAPARAM   stDMA;
} VIQE_HI_TYPE;
/*===========================================================================*/

/*
 * Cortex-M3 Early-view Mode
 */
#define IOCTL_CM3_CTRL_OFF		    0
#define IOCTL_CM3_CTRL_ON			1
#define IOCTL_CM3_CTRL_RESET    	2
#define IOCTL_CM3_CTRL_CMD			3

typedef struct {
    int iOpCode;
    int* pHandle;
    void* pParam1;
    void* pParam2;
} t_cm3_avn_cmd;

typedef enum {
	HW_TIMER_TEST 					= 0x70,
	GET_EARLY_CAMERA_STATUS 		= 0x71,
	SET_EARLY_CAMERA_STOP 			= 0x72,
	SET_EARLY_CAMERA_DISPLAY_OFF 	= 0x73,
	SET_REAR_CAMERA_RGEAR_DETECT	= 0x74,
} CM3_AVN_CMD;

/* vout */
#define MPLANE_NUM	2
#define MPLANE_VID	0
#define MPLANE_SUB	1

enum mplane_vid_component {
	/* video information */
	VID_SRC = 0,			// MPLANE_VID 0x0
	VID_NUM = 1,			// num of mplanes
	VID_BASE1 = 2,			// base1 address of video src (U/Cb)
	VID_BASE2 = 3,			// base1 address of video src (V/Cr)
	VID_WIDTH = 4,			// width/height of video src
	VID_HEIGHT = 5,
	VID_CROP_LEFT = 6,		// crop-[left/top/width/height] of video src
	VID_CROP_TOP = 7,
	VID_CROP_WIDTH = 8,
	VID_CROP_HEIGHT = 9,
};

enum tcc_vout_status {
	TCC_VOUT_IDLE,
	TCC_VOUT_INITIALISING,	// vout driver opened
	TCC_VOUT_RUNNING,		// vout streamon
	TCC_VOUT_STOP,			// vout streamoff
};


typedef struct {
	char dev_name[12];
	pthread_t frame_threads;

    int 						fd;	
	int							preview_width;
	int							preview_height;
	unsigned int				preview_fmt;
	camera_mode					cam_mode;
	
    struct v4l2_capability		vid_cap;
    struct v4l2_format			vid_fmt;
    struct v4l2_streamparm		vid_parm;
    struct v4l2_requestbuffers	vid_buf;

    unsigned char				*buffers[NUM_VIDBUF];

	//LCD
    int							fb_fd0;
    int							mem_len;
    void						*outbuf; //phy_addr

	int			 				rt_mode;

	int							overlay_fd;
	overlay_config_t			overlay_config;

	int							composite_fd;

	pmap_t						pmap_camera;

	int							fb_xres;
	int							fb_yres;

	int							camdev;
	int							display;	// display on

	/* for viqe histogram */
	int				viqe_fd;
	VIQE_HI_TYPE	viqe_type;
	unsigned short	histogram[16];
	int				histogram_excute;

	/* output display device info. */
	int outdisp_dev;
	int output_width, output_height;
	struct tcc_lcdc_image_update_extend outdisp_info;

	/* option */
	int use;
	int use_scaler;
	int timestamp;	// display time-stamp
	int record;		// recording
	FILE *fp;

	/* i2c */
	int i2c_fd;
	char i2c_dev_port[24];
	int i2c_slave_addr;
	int is_send;
	unsigned char i2c_xfer_data[2];
	unsigned char i2c_reg, i2c_val;

	/* auto start */
	int auto_start;

	/* cm3 */
	int cm3_fd;
	t_cm3_avn_cmd cm3_cmd;

	/* vout */
	int use_vout;
	int vout_fd;
	int vout_status;
	struct v4l2_format vout_fmt;
	struct v4l2_format vout_external_sc_fmt;	// not used (need vioc path)
	struct v4l2_requestbuffers	vout_rbuf;
	struct v4l2_buffer vout_qbuf;
	struct v4l2_plane vout_vid_mplane;
	struct v4l2_buffer vout_dqbuf;
} CameraDevice;


extern void 	open_device 			(CameraDevice *self);
extern void 	close_device 			(CameraDevice *self);
extern void 	init_camera_data 		(CameraDevice *self, unsigned int preview_fmt, int opt);
extern int 		camif_get_dev_info 		(CameraDevice *self);
extern int		camif_get_frame			(CameraDevice *self);
extern void 	camif_set_queryctrl		(CameraDevice *self, unsigned int ctrl_id, int value);
extern void 	camif_set_resolution 	(CameraDevice *self, int width, int height);
extern void 	camif_set_overlay		(CameraDevice *self, int buffer_value);
extern void 	camif_start_stream		(CameraDevice *self);
extern void 	camif_stop_stream		(CameraDevice *self);
extern void 	camif_capture			(CameraDevice *self);
extern void  	camif_encode_jpeg		(CameraDevice *self);
extern void 	camif_save 				(CameraDevice *self, unsigned short* filename);

#endif /* __CAM_V4L2_H__ */
