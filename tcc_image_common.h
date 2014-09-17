/******************************************************************************
*
*  (C)Copyright All Rights Reserved by Telechips Inc.
*
*  This material is confidential and shall remain as such.
*  Any unauthorized use, distribution, reproduction is strictly prohibited.
*
*   FileName    : tcc_image_common.h
*   Description :
*   TCC Version 1.0
*   Copyright (c) Telechips, Inc.
*   ALL RIGHTS RESERVED
*******************************************************************************/
/******************************************************************************
* include
******************************************************************************/

#ifndef _IMAGE_HEADER_H_
#define _IMAGE_HEADER_H_

//#include "tcc_virtual_mem.h"
//#include <lcd_resolution.h>

/******************************************************************************
* defines
******************************************************************************/
//#define MEMORY_DEBUG_MESSAGE
#define TCC_TIMER_INCLUDE
//#if defined (TCC_IMAGE_DECODING_INCLUDE) || defined(TCC_IMAGE_DECODING_INCLUDE)
#ifdef TCC_IMAGE_DECODING_INCLUDE
#define TCC_IMAGE_ENCODE_PROCESS
#endif

#ifndef MAX
/* MAX and MIN macros */
#define MAX(x, y) ((x > y) ? x:y)
#endif

#ifndef MIN
#define MIN(x, y) ((x > y) ? y:x)
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifdef PRINTF_DEBUG
#define IMAGE_DEBUG
#endif

#ifdef PRINTF_DEBUG
#ifdef IMAGE_DEBUG
#define PRINTF printf
#endif
#else
#define PRINTF
#endif

#define IMAGE_DEC_BASE_NAME "OMX.tcc.image_decoder"

#define IMAGE_DEC_BMP_NAME "OMX.tcc.image_decoder.bmp"
#define IMAGE_DEC_BMP_ROLE "image_decoder.bmp"

#define IMAGE_DEC_PNG_NAME "OMX.tcc.image_decoder.png"
#define IMAGE_DEC_PNG_ROLE "image_decoder.png"

#define IMAGE_DEC_GIF_NAME "OMX.tcc.image_decoder.gif"
#define IMAGE_DEC_GIF_ROLE "image_decoder.gif"

#define IMAGE_DEC_JPEG_NAME "OMX.tcc.image_decoder.jpg"
#define IMAGE_DEC_JPEG_ROLE "image_decoder.jpg"


#define TCC_IMAGE_EXIFPARSE_INCLUDE
//#define TCC_IMAGE_FRAME_FIT_INCLUDE
#define TCC_IMAGE_BOTHSIDE_MEMORY_INCLUDE


#define MAX_STREAM_BUF (16 * 16) // kbyte unit
#define PHYSICAL_REG_SIZE (1024 * 4)

#define CLIP(x) (((x) < 0) ? 0 : (((x) > 255) ? 255 : (x)))

#define FBDEV_FILENAME "/dev/fb0"

#define IMAGE_DEFAULT_WIDTH DEFAULT_WIDTH
#define IMAGE_DEFAULT_HEIGHT DEFAULT_HEIGHT

#define IMAGE_DEC_WIDTH DEFAULT_WIDTH
#define IMAGE_DEC_HEIGHT DEFAULT_HEIGHT

#define SCALE_LIMIT_WIDTH IMAGE_DEC_WIDTH
#define SCALE_LIMIT_HEIGHT IMAGE_DEC_HEIGHT

#define IMAGE_EVALUATION_INCLUDE
#define ALIGN_2MUL(x) (((x + 1) >> 1) << 1)
#define ALIGN_M2MUL(x) (((x) >> 1) << 1)
#define ALIGN_4MUL(x) (((x + 3) >> 2) << 2)
#define ALIGN_M4MUL(x) (((x) >> 2) << 2)
#define ALIGN_8MUL(x) (((x + 7) >> 3) << 3)
#define ALIGN_M8MUL(x) (((x) >> 3) << 3)
#define ALIGN_16MUL(x) (((x + 15) >> 4) << 4)
#define ALIGN_M16MUL(x) (((x) >> 4) << 4)
#define ALIGN_32MUL(x) (((x + 31) >> 5) << 5)
#define ALIGN_MM32MUL(x) ((x) <= 32) ? 32 : (((x) >> 5) << 5)
#define ALIGN_M32MUL(x) (((x) >> 5) << 5)
#define ALIGN_64MUL(x) (((x + 63) >> 6) << 6)
#define ALIGN_4K(x) ((x + 4095) & ~(4095))

#define OUTPUT_DECODED_COLOR_FMT OMX_COLOR_FormatYUV420Planar

/******************************************************************************
* typedefs & structure
******************************************************************************/
enum
{
	CODEC_JPEG = 0,
	CODEC_BMP,
	CODEC_PNG,
	CODEC_GIF,
#ifdef TCC_DLNA_STREAM_INCLUDE
	CODEC_JPEG_DLNA_MED,
	CODEC_JPEG_DLNA_SM,
	CODEC_JPEG_DLNA_TN,
#endif
	PHOTO_NUMCODECS
};

enum
{
	ZOOM_IN_LEV4 = 0,
	ZOOM_IN_LEV3,
	ZOOM_IN_LEV2,
	ZOOM_IN_LEV1,
	ZOOM_NONE,
	ZOOM_OUT_LEV1,
	ZOOM_OUT_LEV2,
	ZOOM_OUT_LEV3,
	ZOOM_OUT_LEV4,
	ZOOM_VALUE_MAX
};

enum
{
	ROTATE_NONE = 0,
	ROTATE_NONE_R1, //90
	ROTATE_NONE_R2, //180
	ROTATE_NONE_R3, //270
	ROTATE_VALUE_MAX
};

enum
{
	PHY_MEMORY_NO_ERROR = 0,
	PHY_MEMORY_OPEN_ERR,
	PHY_MEMORY_MALLOC_ERR,
	PHY_MEMORY_REG_ERROR
};

enum
{
	IMAGE_FORMAT_YUV420Sep,
	IMAGE_FORMAT_YUV422Sep,
	IMAGE_FORMAT_YUV422Seq, /* yuv 422 Sequential */
	IMAGE_FORMAT_YUV444,
	IMAGE_FORMAT_RGB565
};

enum
{
	DEC_OUT_FORMAT_YUV = 0,
	DEC_OUT_FORMAT_RGB,
	DEC_OUT_FORMAT_RGB888
};

enum
{
	IMAGE_INFO_FORMAT_RGB = 0,
	IMAGE_INFO_FORMAT_YUV,
	IMAGE_INFO_FORMAT_RGB888
};

enum
{
	IMAGE_APP_ERROR_NONE = 0,
	IMAGE_APP_ERROR_SCALELIMIT,
	IMAGE_APP_ERROR_DECINIT,
	IMAGE_APP_ERROR_FRAMEMODE_SCALE,
	IMAGE_APP_ERROR_CANT_MOVE
};

typedef struct SW_JPEG_DEC_PARAMTYPE
{
	int width;
	int height;
	int out_format;
	int start_offset;
} SW_JPEG_DEC_PARAMTYPE;

typedef struct IMAGE_DEC_COMMON_PARAMTYPE
{
	int width;
	int height;
	int out_format;
	int start_offset;
	int flow;
	int hwscale;
} IMAGE_DEC_COMMON_PARAMTYPE;


#endif
