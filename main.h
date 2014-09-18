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
#include <mach/vioc_global.h>

#include "v4l2.h"
#include "rsc.h"

#include "pmap.h"


/*=============================================================================

                                 FEATURE 

=============================================================================*/
#define CAMERA_DEVICE_NAME 		"/dev/video0"
#define FB_DEVICE_NAME			"/dev/fb0"
#define OVERLAY_DEVICE_NAME		"/dev/overlay"
#define COMPOSITE_DEVICE 		"/dev/composite"
#define VIQE_DEVICE_NAME		"/dev/viqe"

#define PREVIEW_WIDTH	 800
#define PREVIEW_HEIGHT	 600

extern int cam_id;
#define DEVICE_NR	5

/*
 * Cortex-M3 Early-view Mode
 */
#define CM3_EARLY_VIEW
#ifdef CM3_EARLY_VIEW
#define CM3_DRV_NAME	"/dev/tcc_cm3_ctrl"
#endif

/*
 * V4L2 Video Ouptput (vout)
 */
#define VOUT_DRV_NAME	"/dev/video10"

/** 
 * Selection Preview Image Pixel Format 
 */
static inline unsigned int vioc2fourcc(unsigned int vioc_fmt)
{
	unsigned int fourcc;
	switch(vioc_fmt) {
	/* sequential (YUV packed) */
	case VIOC_IMG_FMT_UYVY:				// LSB [Y/U/Y/V] MSB : YCbCr 4:2:2 sequential
		fourcc = V4L2_PIX_FMT_UYVY;			// 'UYVY' 16 YUV 4:2:2
		break;
	case VIOC_IMG_FMT_VYUY:				// LSB [Y/V/Y/U] MSB : YCbCr 4:2:2 sequential
		fourcc = V4L2_PIX_FMT_VYUY;			// 'VYUY' 16 YUV 4:2:2
		break;
	case VIOC_IMG_FMT_YUYV:				// LSB [Y/U/Y/V] MSB : YCbCr 4:2:2 sequential
		fourcc = V4L2_PIX_FMT_YUYV;			// 'YUYV' 16 YUV 4:2:2
		break;
	case VIOC_IMG_FMT_YVYU:				// LSB [Y/V/Y/U] MSB : YCbCr 4:2:2 sequential
		fourcc = V4L2_PIX_FMT_YVYU;			// 'YVYU' 16 YVU 4:2:2
		break;

	/* sepatated (Y, U, V planar) */
	case VIOC_IMG_FMT_YUV420SEP:		// YCbCr 4:2:0 separated
		fourcc = V4L2_PIX_FMT_YVU420;		// 'YV12' 12 YVU 4:2:0
		break;
	case VIOC_IMG_FMT_YUV422SEP:		// YCbCr 4:2:2 separated
		fourcc = V4L2_PIX_FMT_YUV422P;		// '422P' 16 YVU422 Planar
		break;

	/* interleaved (Y palnar, UV planar) */
	case VIOC_IMG_FMT_YUV420IL0:		// YCbCr 4:2:0 interleaved type0
		fourcc = V4L2_PIX_FMT_NV12;			// 'NV12' 12 Y/CbCr 4:2:0
		break;
	case VIOC_IMG_FMT_YUV420IL1:		// YCbCr 4:2:0 interleaved type1
		fourcc = V4L2_PIX_FMT_NV21;			// 'NV21' 12 Y/CrCb 4:2:0
		break;
	case VIOC_IMG_FMT_YUV422IL0:		// YCbCr 4:2:2 interleaved type0
		fourcc = V4L2_PIX_FMT_NV16;			// 'NV16' 16 Y/CbCr 4:2:2
		break;
	case VIOC_IMG_FMT_YUV422IL1:		// YCbCr 4:2:2 interleaved type1
		fourcc = V4L2_PIX_FMT_NV61;			// 'NV61' 16 Y/CrCb 4:2:2
		break;

	/* RGB */
	case VIOC_IMG_FMT_RGB332:			// R[7:5] G[4:2] B[1:0]
		fourcc = V4L2_PIX_FMT_RGB332;		// 'RGB1' 8 RGB-3-3-2
		break;
	case VIOC_IMG_FMT_ARGB4444:			// A[15:12] R[11:8] G[7:4] B[3:0]
		fourcc = V4L2_PIX_FMT_RGB444;		// 'R444' 16 RGB-4-4-4 (xxxxrrrr ggggbbbb)
		break;
	case VIOC_IMG_FMT_ARGB1555:			// A[15] R[14:10] G[9:5] B[4:0]
		fourcc = V4L2_PIX_FMT_RGB555;		// 'RGB0' 16 RGB-5-5-5
		break;
	case VIOC_IMG_FMT_RGB565:			// R[15:11] G[10:5] B[4:0]
		fourcc = V4L2_PIX_FMT_RGB565;		// 'RGBP' 16 RGB-5-6-5
		break;
	case VIOC_IMG_FMT_RGB888:			// B1[31:24] R[23:16] G[15:8] B0[7:0]
		fourcc = V4L2_PIX_FMT_RGB24;		// 'RBG3' 24 RGB-8-8-8
		//fourcc = V4L2_PIX_FMT_BGR24;		// 'BGR3' 24 BGR-8-8-8
		break;
	case VIOC_IMG_FMT_ARGB8888:			// A[31:24] R[23:16] G[15:8] B[7:0]
		fourcc = V4L2_PIX_FMT_RGB32;		// 'RGB4' 32 RGB-8-8-8-8
		//fourcc = V4L2_PIX_FMT_BGR32;		// 'BGR4' 32 BGR-8-8-8-8
		break;

	default:
		fourcc = V4L2_PIX_FMT_YVU420;
		break;
	}

	return fourcc;
}

#endif

