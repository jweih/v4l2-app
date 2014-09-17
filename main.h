/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

               Camera    I n t e r f a c e    M O D U L E

                        EDIT HISTORY FOR MODULE

when        who       what, where, why
--------    ---       -------------------------------------------------------
10/xx/08   ZzaU      Created file.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/


#ifndef __CAM_MAIN_H__
#define __CAM_MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/types.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/kd.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/poll.h>
#include <getopt.h>
#include <termios.h>
#include <ctype.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <linux/videodev2.h>
#include <mach/tcc_cam_ioctrl.h>
#include <mach/tccfb_ioctrl.h>
#include <mach/tcc_composite_ioctl.h>
#include <mach/tcc_component_ioctl.h>

#include "v4l2.h"
#include "rsc.h"

#include "pmap.h"


/*=============================================================================

                                 FEATURE 

=============================================================================*/

//#define _PUSH_VSYNC_

#if defined(_PUSH_VSYNC_)
#define _PUSH_VSYNC_TT_TEST_00_
#endif

#define CAMERA_DEVICE_NAME 		"/dev/video0"
#define FB_DEVICE_NAME			"/dev/fb0"

#ifndef _PUSH_VSYNC_
#define OVERLAY_DEVICE_NAME		"/dev/overlay"
#endif

#define COMPOSITE_DEVICE 		"/dev/composite"

#define VIQE_DEVICE_NAME		"/dev/viqe"

#if defined(_PUSH_VSYNC_TT_TEST_00_)
#define PREVIEW_WIDTH	 720
#define PREVIEW_HEIGHT	 480
#else
#define PREVIEW_WIDTH	 800
#define PREVIEW_HEIGHT	 600
#endif

#define DBug_printf printf

extern int debug_flg;
extern int cam_id;

#define DEVICE_NR	5

/** 
 * Selection Preview Image Pixel Format 
 */
static inline unsigned int vioc2fourcc(unsigned int vioc_fmt)
{
	unsigned int fourcc;
	switch(vioc_fmt) {
	/* sequential (YUV packed) */
	case TCC_LCDC_IMG_FMT_UYVY:			// LSB [Y/U/Y/V] MSB : YCbCr 4:2:2 sequential
		fourcc = V4L2_PIX_FMT_UYVY;			// 'UYVY' 16 YUV 4:2:2
		break;
	case TCC_LCDC_IMG_FMT_VYUY:			// LSB [Y/V/Y/U] MSB : YCbCr 4:2:2 sequential
		fourcc = V4L2_PIX_FMT_VYUY;			// 'VYUY' 16 YUV 4:2:2
		break;
	case TCC_LCDC_IMG_FMT_YUYV:			// LSB [Y/U/Y/V] MSB : YCbCr 4:2:2 sequential
		fourcc = V4L2_PIX_FMT_YUYV;			// 'YUYV' 16 YUV 4:2:2
		break;
	case TCC_LCDC_IMG_FMT_YVYU:			// LSB [Y/V/Y/U] MSB : YCbCr 4:2:2 sequential
		fourcc = V4L2_PIX_FMT_YVYU;			// 'YVYU' 16 YVU 4:2:2
		break;

	/* sepatated (Y, U, V planar) */
	case TCC_LCDC_IMG_FMT_YUV420SP:		// YCbCr 4:2:0 separated
		fourcc = V4L2_PIX_FMT_YVU420;		// 'YV12' 12 YVU 4:2:0
		break;
	case TCC_LCDC_IMG_FMT_YUV422SP:		// YCbCr 4:2:2 separated
		fourcc = V4L2_PIX_FMT_YUV422P;		// '422P' 16 YVU422 Planar
		break;

	/* interleaved (Y palnar, UV planar) */
	case TCC_LCDC_IMG_FMT_YUV420ITL0:	// YCbCr 4:2:0 interleaved type0
		fourcc = V4L2_PIX_FMT_NV12;			// 'NV12' 12 Y/CbCr 4:2:0
		break;
	case TCC_LCDC_IMG_FMT_YUV420ITL1:	// YCbCr 4:2:0 interleaved type1
		fourcc = V4L2_PIX_FMT_NV21;			// 'NV21' 12 Y/CrCb 4:2:0
		break;
	case TCC_LCDC_IMG_FMT_YUV422ITL0:	// YCbCr 4:2:2 interleaved type0
		fourcc = V4L2_PIX_FMT_NV16;			// 'NV16' 16 Y/CbCr 4:2:2
		break;
	case TCC_LCDC_IMG_FMT_YUV422ITL1:	// YCbCr 4:2:2 interleaved type1
		fourcc = V4L2_PIX_FMT_NV61;			// 'NV61' 16 Y/CrCb 4:2:2
		break;

	default:
		fourcc = V4L2_PIX_FMT_YVU420;
		break;
	}

	return fourcc;
}

#endif

