/****************************************************************************
 *   FileName    : tcc_scaler_interface.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/
#ifndef __TCC_SCALER_INTERFACE_H_
#define __TCC_SCALER_INTERFACE_H_

#if (1)//tt
#include <mach/tcc_scaler_ioctrl.h>
#else
#include "tcc_scaler_ioctrl.h"
#endif
#include "user_debug_levels.h"

#define TCC_SCALER_DEV0_NAME 		"/dev/scaler"
#define TCC_SCALER_DEV1_NAME 		"/dev/scaler1"

typedef struct tagScalerInfo
{
	//image address
	char* addr_y;
	char* addr_u;
	char* addr_v;

	//image size
	long x;
	long y;
	long w;
	long h;

	//window size
	long w_x;
	long w_y;
	long w_w;
	long w_h;

	//image format
	int format;
}tcc_scaler_info_t;


enum{
	SCALER_MEMORY_NO_ERROR = 0,
	SCALER_MEMORY_OPEN_ERR,	
	SCALER_MEMORY_MALLOC_ERR,
	SCALER_MEMORY_REG_ERROR, 
	SCALER_INTERRUPT_OPEN_ERROR, 
	SCALER_INTERRUPT_POLL_ERROR, 		
	SCALER_INTERRUPT_POLLTIME_ERROR, 	
	SCALER_INTERRUPT_POLLERR_ERROR 	
};

int tcc_scaler_open(const char* dev_name);

int tcc_scaler_execute(const int fd, tcc_scaler_info_t* src, tcc_scaler_info_t* dst);

int tcc_scaler_close(const int fd);




int  tcc_scaler_work(char * src,char* phy_src, char * tgt,char* phy_tgt,
						int src_buffer_w,int src_buffer_h,int src_image_w,int src_image_h,int src_frame_w,int src_frame_h,
						int dst_buffer_w,int dst_buffer_h,int dst_image_w,int dst_image_h,int dst_frame_w,int dst_frame_h,
						int src_y_off,int src_u_off,int src_v_off,int dst_y_off,int dst_u_of,int dst_v_off,int full, int format,
						int s_x_off,int s_y_off,int d_x_off, int d_y_off);



int tcc_scaler_yuv420_full( unsigned int s_hsize, unsigned int s_vsize, unsigned int so_hsize,unsigned int so_vsize,
					    unsigned int m_hsize, unsigned int m_vsize,unsigned int do_hsize,unsigned int do_vsize,
	                    unsigned int s_y, unsigned int s_u, unsigned int s_v,
	                    unsigned int d_y, unsigned int d_u, unsigned int d_v);

int tcc_scaler_yuv420( unsigned int s_hsize, unsigned int s_vsize, unsigned int so_hsize,unsigned int so_vsize,
				       unsigned int m_hsize, unsigned int m_vsize,unsigned int do_hsize,unsigned int do_vsize,
                       unsigned int s_y, unsigned int s_u, unsigned int s_v,
                       unsigned int d_y, unsigned int d_u, unsigned int d_v,
                       unsigned int s_x_off,unsigned int s_y_off,unsigned int d_x_off, unsigned int d_y_off) ;


int tcc_scaler_rgb565( unsigned int s_hsize, unsigned int s_vsize, unsigned int so_hsize,unsigned int so_vsize,
				       unsigned int m_hsize, unsigned int m_vsize,unsigned int do_hsize,unsigned int do_vsize,
                       unsigned int s_y, unsigned int s_u, unsigned int s_v,
                       unsigned int d_y, unsigned int d_u, unsigned int d_v);

int tcc_scaler_yuv422( unsigned int s_hsize, unsigned int s_vsize, unsigned int so_hsize,unsigned int so_vsize,
				       unsigned int m_hsize, unsigned int m_vsize,unsigned int do_hsize,unsigned int do_vsize,
                       unsigned int s_y, unsigned int s_u, unsigned int s_v,
                       unsigned int d_y, unsigned int d_u, unsigned int d_v);

int tcc_scaler_yuv422seq( unsigned int s_hsize, unsigned int s_vsize, unsigned int so_hsize,unsigned int so_vsize,
				       unsigned int m_hsize, unsigned int m_vsize,unsigned int do_hsize,unsigned int do_vsize,
                       unsigned int s_y, unsigned int s_u, unsigned int s_v,
                       unsigned int d_y, unsigned int d_u, unsigned int d_v,
                       unsigned int s_x_off,unsigned int s_y_off,unsigned int d_x_off, unsigned int d_y_off,
                       int src_format,int dst_format,int full);


int tcc_camera_scaler_open(const char* dev_name);

int tcc_camera_scaler_execute(const int fd, tcc_scaler_info_t* src, tcc_scaler_info_t* dst);
int tcc_camera_scaler_execute_2port_vetical(const int fd, tcc_scaler_info_t* src, tcc_scaler_info_t* dst, int PortIndex);
int tcc_camera_scaler_execute_2port_horizontal(const int fd, tcc_scaler_info_t* src, tcc_scaler_info_t* dst, int PortIndex);
int tcc_camera_scaler_execute_4port(const int fd, tcc_scaler_info_t* src, tcc_scaler_info_t* dst, int PortIndex);

int tcc_camera_scaler_close(const int fd);

#endif /* __TCC_SCALER_INTERFACE_H_ */
