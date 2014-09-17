#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <stdio.h>
#include "tcc_image_common.h"
//#include <mach/tcc_component_ioctl.h>

#include "tcc_scaler_interface.h"


/* FILT2D IOCTL code */
#define TCC_F2D_SET_MODE				0x301
#define TCC_F2D_SET_LPF_STRENGTH		0x302
#define TCC_F2D_SET_HPF_STRENGTH		0x303
#define TCC_F2D_SET_3x3WIN_COEFF_CH0	0x304
#define TCC_F2D_SET_3x3WIN_COEFF_CH1	0x305
#define TCC_F2D_SET_3x3WIN_COEFF_CH2	0x306
#define TCC_F2D_SET_DIV					0x307
#define TCC_F2D_SET_ENABLE				0x3FF
//

#define	PLANE0		0
#define	PLANE1		1
#define	PLANE2		2

#define	SMODE_00	0
#define	SMODE_01	1
#define	SMODE_02	2

#define ON		1
#define OFF		0

typedef struct {
	/* set detecting directon filter mode */
	// 0: LPF mode
	// 1: HPF mode
	unsigned int hpf0_en;		// channel.0
	unsigned int hpf1_en;		// channel.1
	unsigned int hpf2_en;		// channel.2

	/* set bypass mode */
	// 0: Normal mode
	// 1: Bypass mode
	unsigned int bypass0_en;	// channel.0
	unsigned int bypass1_en;	// channel.1
	unsigned int bypass2_en;	// channel.2

	/* set filter mode */
	// 0: detecting direction filter mode
	// 1: 3x3 window filter mode
	unsigned int simple0_mode;	// channel.0
	unsigned int simple1_mode;	// channel.1
	unsigned int simple2_mode;	// channel.2
} F2D_MODE_PARAM;

enum F2D_STRENGTH_TYPE {
	STRENGTH_1 = 0,
	STRENGTH_2 = 1,
	STRENGTH_3 = 2,
	STRENGTH_4 = 3,
	STRENGTH_5 = 4,
	STRENGTH_0 = 5,	// bypass
};

typedef struct {
	enum F2D_STRENGTH_TYPE ch0;
	enum F2D_STRENGTH_TYPE ch1;
	enum F2D_STRENGTH_TYPE ch2;
} F2D_FILT_STRENGTH_PARM;

typedef struct {
	unsigned int para[9];	// [0]-P00, [1]-P01, ..., [7]-P21, [8]-P22
	unsigned int cb;
} F2D_SCOEFF_PARAM;

typedef struct {
	unsigned int pos;
	unsigned int dtog;
	unsigned int den;
} F2D_DIV_PARAM;


void filt2d_coeff_lpf (int fd, unsigned int strength0, unsigned int strength1, unsigned int strength2)
{
	F2D_FILT_STRENGTH_PARM f2d_lpf_strength;

	f2d_lpf_strength.ch0 = strength0;
	f2d_lpf_strength.ch1 = strength1;
	f2d_lpf_strength.ch2 = strength2;

	ioctl(fd, TCC_F2D_SET_LPF_STRENGTH, &f2d_lpf_strength);
}

void filt2d_coeff_hpf (int fd, unsigned int strength0, unsigned int strength1, unsigned int strength2)
{
	F2D_FILT_STRENGTH_PARM f2d_hpf_strength;

	f2d_hpf_strength.ch0 = strength0;
	f2d_hpf_strength.ch1 = strength1;
	f2d_hpf_strength.ch2 = strength2;

	ioctl(fd, TCC_F2D_SET_HPF_STRENGTH, &f2d_hpf_strength);
}

void filt2d_coeff_simple (int fd, unsigned int plane, unsigned int *scoeff)
{
	F2D_SCOEFF_PARAM f2d_scoeff;

	memcpy(&f2d_scoeff, scoeff, sizeof(F2D_SCOEFF_PARAM));
	switch(plane)
	{
		case PLANE0:
			ioctl(fd, TCC_F2D_SET_3x3WIN_COEFF_CH0, &f2d_scoeff);
			break;
		case PLANE1:
			ioctl(fd, TCC_F2D_SET_3x3WIN_COEFF_CH1, &f2d_scoeff);
			break;
		case PLANE2:
			ioctl(fd, TCC_F2D_SET_3x3WIN_COEFF_CH2, &f2d_scoeff);
			break;
	}
}

void filt2d_mode (int fd, unsigned int hpf0_en, unsigned int hpf1_en, unsigned int hpf2_en,
						unsigned int bypass0_en, unsigned int bypass1_en, unsigned int bypass2_en,
						unsigned int simple0_mode, unsigned int simple1_mode, unsigned int simple2_mode)
{
	F2D_MODE_PARAM f2d_mode;

	f2d_mode.hpf0_en = hpf0_en;
	f2d_mode.hpf1_en = hpf1_en;
	f2d_mode.hpf2_en = hpf2_en;
	f2d_mode.bypass0_en = bypass0_en;
	f2d_mode.bypass1_en = bypass1_en;
	f2d_mode.bypass2_en = bypass2_en;
	f2d_mode.simple0_mode = simple0_mode;
	f2d_mode.simple1_mode = simple1_mode;
	f2d_mode.simple2_mode = simple2_mode;

	ioctl(fd, TCC_F2D_SET_MODE, &f2d_mode);
}

void filt2d_div	(int fd, uint pos, uint dtog, uint den)
{
	F2D_DIV_PARAM f2d_div;

	f2d_div.pos  = pos;
	f2d_div.dtog = dtog;
	f2d_div.den  = den;

	ioctl(fd, TCC_F2D_SET_DIV, &f2d_div);
}

void filt2d_enable (int fd, unsigned int enable)
{
	ioctl(fd, TCC_F2D_SET_ENABLE, &enable);
}

/*simple 3x3 window filter coefficient example. */
static unsigned int f2d_simple_coeff[3][10] = {  
  //|P00  P01  P02| P10  P11  P12| P20  P21  P22|  CB
	{0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0,  0x0}, 
	{0x1, 0x2, 0x1, 0x2, 0x4, 0x2, 0x1, 0x2, 0x1,  0x4},
	{0x1, 0x1, 0x1, 0x1, 0x8, 0x1, 0x1, 0x1, 0x1,  0x4},
};

extern unsigned int f2d_type;
extern unsigned int f2d_opt;
void filt2d_test(int sc_fd)
{
	static int first_run = 1;

	if(first_run)
	{
		filt2d_enable(sc_fd, OFF);
		first_run = 0;
	}
	
	#if 0
	filt2d_coeff_lpf	(sc_fd, STRENGTH_3, STRENGTH_3, STRENGTH_3);
	filt2d_coeff_hpf	(sc_fd, STRENGTH_3, STRENGTH_3, STRENGTH_3);
	filt2d_coeff_simple	(sc_fd, PLANE0, &f2d_simple_coeff[SMODE_01]);
	filt2d_coeff_simple	(sc_fd, PLANE1, &f2d_simple_coeff[SMODE_01]);
	filt2d_coeff_simple	(sc_fd, PLANE2, &f2d_simple_coeff[SMODE_01]);
	filt2d_mode			(sc_fd, OFF, OFF, OFF, ON, ON, ON, ON, ON, ON);
	filt2d_div			(sc_fd, 0, 0, OFF);
	filt2d_enable		(sc_fd, ON);
	filt2d_enable		(sc_fd, OFF);
	#endif
	switch(f2d_type)
	{
		case 0: //none.
			filt2d_enable		(sc_fd, OFF);
			break;
		case 1:	//HPF
			filt2d_coeff_hpf	(sc_fd, f2d_opt, f2d_opt, f2d_opt);
			filt2d_mode			(sc_fd, ON, ON, ON, OFF, OFF, OFF, OFF, OFF, OFF);		// HPF test
			filt2d_div			(sc_fd, 120, 0, ON);
			filt2d_enable		(sc_fd, ON);
			break;
		case 2:	//LPF
			filt2d_coeff_lpf	(sc_fd, f2d_opt, f2d_opt, f2d_opt);
			filt2d_mode			(sc_fd, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF);	// LPF test
			filt2d_div			(sc_fd, 120, 0, ON);
			filt2d_enable		(sc_fd, ON);
			break;
		case 3:	//SM
			filt2d_coeff_simple	(sc_fd, PLANE0, &f2d_simple_coeff[f2d_opt][0]);
			filt2d_coeff_simple	(sc_fd, PLANE1, &f2d_simple_coeff[f2d_opt][0]);
			filt2d_coeff_simple	(sc_fd, PLANE2, &f2d_simple_coeff[f2d_opt][0]);
			filt2d_mode			(sc_fd, OFF, OFF, OFF, OFF, OFF, OFF, ON, ON, ON);		// Simple 3x3 Window Filter test
			filt2d_div			(sc_fd, 120, 0, ON);
			filt2d_enable		(sc_fd, ON);
			break;
		default:
			filt2d_enable		(sc_fd, OFF);
			break;

	}

}

void filt2d_main(void)
{
#if 1
	int sc_fd;

	// scaler open.
	sc_fd = ("/dev/scaler", O_RDWR | O_NDELAY);

	// filt2d setting.
	filt2d_coeff_lpf	(sc_fd, STRENGTH_3, STRENGTH_3, STRENGTH_3);
	filt2d_coeff_hpf	(sc_fd, STRENGTH_3, STRENGTH_3, STRENGTH_3);
	filt2d_coeff_simple	(sc_fd, PLANE0, &f2d_simple_coeff[SMODE_01][0]);
	filt2d_coeff_simple	(sc_fd, PLANE1, &f2d_simple_coeff[SMODE_01][0]);
	filt2d_coeff_simple	(sc_fd, PLANE2, &f2d_simple_coeff[SMODE_01][0]);
	filt2d_mode			(sc_fd, OFF, OFF, OFF, ON, ON, ON, ON, ON, ON);
	filt2d_div			(sc_fd, 0, 0, OFF);
	filt2d_enable		(sc_fd, ON);

	// scaler setting.
	// ...
	
	// scaler execute.
	// ...

	// scaler close.
	fclose(sc_fd);
	
	
#else
	int sc_fd;

	// scaler open.
	fd = ("/dev/scaler", O_RDWR | O_NDELAY);
	
	F2D_MODE_PARAM f2d_mode;
	F2D_FILT_STRENGTH_PARM f2d_lpf_strength;
	F2D_FILT_STRENGTH_PARM f2d_hpf_strength;
	F2D_SCOEFF_PARAM *f2d_scoeff[3];
	F2D_DIV_PARAM f2d_div;
	unsigned int f2d_en;

	f2d_lpf_strength.ch0 = STRENGTH_3;
	f2d_lpf_strength.ch1 = STRENGTH_3;
	f2d_lpf_strength.ch2 = STRENGTH_3;

	f2d_hpf_strength.ch0 = STRENGTH_3;
	f2d_hpf_strength.ch1 = STRENGTH_3;
	f2d_hpf_strength.ch2 = STRENGTH_3;

	f2d_scoeff[0] = f2d_simple_coeff[SMODE_01];
	f2d_scoeff[1] = f2d_simple_coeff[SMODE_01];
	f2d_scoeff[2] = f2d_simple_coeff[SMODE_01];
	
	f2d_mode.hpf0_en = OFF;
	f2d_mode.hpf1_en = OFF;
	f2d_mode.hpf2_en = OFF;
	f2d_mode.bypass0_en = ON;
	f2d_mode.bypass1_en = ON;
	f2d_mode.bypass2_en = ON;
	f2d_mode.simple0_mode = ON;
	f2d_mode.simple1_mode = ON;
	f2d_mode.simple2_mode = ON;
	
	f2d_div.pos  = 0;
	f2d_div.dtog = 1;
	f2d_div.den  = OFF;

	f2d_en = ON;

	ioctl(sc_fd, TCC_F2D_SET_LPF_STRENGTH, &f2d_lpf_strength);
	ioctl(sc_fd, TCC_F2D_SET_HPF_STRENGTH, &f2d_hpf_strength);
	ioctl(sc_fd, TCC_F2D_SET_3x3WIN_COEFF_CH0, f2d_scoeff[0]);
	ioctl(sc_fd, TCC_F2D_SET_3x3WIN_COEFF_CH1, f2d_scoeff[1]);
	ioctl(sc_fd, TCC_F2D_SET_3x3WIN_COEFF_CH2, f2d_scoeff[2]);
	ioctl(sc_fd, TCC_F2D_SET_MODE, &f2d_mode);
	ioctl(sc_fd, TCC_F2D_SET_ENABLE, &f2d_en);

	// scaler setting.
	// ...
	
	// scaler execute.
	// ...

	// scaler close.
	fclose(sc_fd);
#endif
}






/*****************************************************************************
* Function Name : 
******************************************************************************
* Desription  : tcc_video_scaler_open
* Parameter   : dev_name:device driver name 
* Return	  : error:-1, others file descriptor
******************************************************************************/
int tcc_scaler_open(const char* dev_name)
{
	int fd = -1;

	if(dev_name != NULL)
	{
		fd = open(dev_name, O_RDWR | O_NDELAY);
		if(fd < 0)
			DEBUG (DEB_LEV_ERR, "can't open %s\n", dev_name);
	}

	return fd;
}

/*****************************************************************************
* Function Name : 
******************************************************************************
* Desription  : tcc_video_scaler_execute
* Parameter   : fd:file descriptor

				src: addr_y, addr_u, addr_v is src address 
					 x,y,w,h is input image size
					 w_x, w_y, w_w, w_h is window size (it can be used for zoom-in)

				dst: 
					addr_y, addr_u, addr_v is dest address 
					x,y,w,h is destination size
				
* Return	  : error:-1, others succeed
******************************************************************************/
int tcc_scaler_execute(const int fd, tcc_scaler_info_t* src, tcc_scaler_info_t* dst)
{
	int ret;
	struct pollfd poll_event[1];
	SCALER_TYPE scaler_info;

	if(fd < 0)
		return -1;
		
	scaler_info.responsetype 		= SCALER_INTERRUPT; 
	
	scaler_info.src_Yaddr			= src->addr_y;
	scaler_info.src_Uaddr			= src->addr_u;
	scaler_info.src_Vaddr			= src->addr_v;
	scaler_info.src_fmt				= src->format;
	scaler_info.src_ImgWidth 		= src->w;		
	scaler_info.src_ImgHeight		= src->h;	
	
	scaler_info.src_winLeft			= src->w_x;
	scaler_info.src_winTop			= src->w_y;
	scaler_info.src_winRight 		= src->w_x + src->w_w;		
	scaler_info.src_winBottom		= src->w_y + src->w_h;		

	scaler_info.dest_Yaddr			= dst->addr_y;
	scaler_info.dest_Uaddr			= dst->addr_u;
	scaler_info.dest_Vaddr			= dst->addr_v;
	
	scaler_info.dest_fmt 			= dst->format; 	
	scaler_info.dest_ImgWidth		= dst->w;
	scaler_info.dest_ImgHeight		= dst->h;
	
	scaler_info.dest_winLeft 		= dst->x;
	scaler_info.dest_winTop			= dst->y;
 	scaler_info.dest_winRight		= dst->x + dst->w;
	scaler_info.dest_winBottom		= dst->y + dst->h;
	
  	scaler_info.viqe_onthefly = 0;
  	scaler_info.interlaced = 0;
  
	if(ioctl(fd, TCC_SCALER_IOCTRL, &scaler_info) != 0)
	{
		DEBUG (DEB_LEV_ERR, "scaler error\n");
		close(fd);
		return -1;
	}

	if(scaler_info.responsetype == SCALER_INTERRUPT)
	{
		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd = fd;
		poll_event[0].events = POLLIN;
		ret = poll((struct pollfd*)poll_event, 1, 400);

		if(ret < 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if(ret == 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if(ret > 0) 
		{
			if (poll_event[0].revents & POLLERR) 
			{
				DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
				return -1;
			}
		}
	}

	return 0;
}

/*****************************************************************************
* Function Name : 
******************************************************************************
* Desription  : tcc_video_scaler_close
* Parameter   : fd:file descriptor
* Return	  : error:-1, others succeed
******************************************************************************/
int tcc_scaler_close(const int fd)
{
	if(fd >= 0)
	{
		close(fd);
		return 0;	
	}
	else
		DEBUG (DEB_LEV_ERR, "video scaler close error\n");
		
	return -1;
}

/*****************************************************************************
* Function Name : tcc_scaler_yuv420_full
******************************************************************************
* Desription  : 
* Parameter   :
* Return      :
******************************************************************************/

int  tcc_scaler_work(char * src,char* phy_src, char * tgt,char* phy_tgt,
						int src_buffer_w,int src_buffer_h,int src_image_w,int src_image_h,int src_frame_w,int src_frame_h,
						int dst_buffer_w,int dst_buffer_h,int dst_image_w,int dst_image_h,int dst_frame_w,int dst_frame_h,
						int src_y_off,int src_u_off,int src_v_off,int dst_y_off,int dst_u_of,int dst_v_off,int full, int format,
						int s_x_off,int s_y_off,int d_x_off, int d_y_off)
{	
	int ret = SCALER_MEMORY_NO_ERROR;
	int i;

	#if 1
	if (src_image_w < (dst_image_w >> 2) || src_image_h < (dst_image_h >> 3))
	{
		// because of the scaler limit
		src_frame_w = src_image_w;
		src_frame_h = src_image_h;

		if (src_image_w < (dst_image_w >> 2))
			dst_image_w = src_image_w * 2;

		if (src_image_h < (dst_image_h >> 3))
			dst_image_h = src_image_h * 3;

		d_x_off = (dst_frame_w / 2) - (dst_image_w / 2);
		d_y_off = (dst_frame_h / 2) - (dst_image_h / 2);
	}	
	#endif

	switch(format)
	{
		case IMAGE_FORMAT_YUV422Seq:
			for(i = 0 ; i < dst_buffer_w*dst_buffer_h ; i++)
			{
				*tgt++ = 0x00;
				*tgt++ = 0x80;
			}
			ret = tcc_scaler_yuv422seq(src_frame_w,src_frame_h,src_image_w, src_image_h, dst_image_w, dst_image_h,dst_frame_w,dst_frame_h,
				phy_src+src_y_off, phy_src+src_buffer_w*src_buffer_h+src_u_off,phy_src+src_buffer_w*src_buffer_h+src_buffer_w*src_buffer_h/4+src_v_off, 
				phy_tgt+dst_y_off, phy_tgt+dst_buffer_w*dst_buffer_h+dst_u_of , phy_tgt+dst_buffer_w*dst_buffer_h+dst_buffer_w*dst_buffer_h/4+dst_v_off,
				s_x_off,s_y_off,d_x_off,d_y_off,SCALER_YUV420_sp,SCALER_YUV422_sq0,full);
			break;
	
		case IMAGE_FORMAT_YUV420Sep:
			memset(tgt, 0x00, dst_buffer_w*dst_buffer_h);
			memset(tgt+dst_buffer_w*dst_buffer_h, 0x80, dst_buffer_w*dst_buffer_h);

			if(full)
			{
				ret = tcc_scaler_yuv420_full(src_frame_w,src_frame_h,src_image_w, src_image_h, dst_image_w, dst_image_h,dst_frame_w,dst_frame_h,
					phy_src+src_y_off, phy_src+src_buffer_w*src_buffer_h+src_u_off,phy_src+src_buffer_w*src_buffer_h+src_buffer_w*src_buffer_h/4+src_v_off, 
					phy_tgt+dst_y_off, phy_tgt+dst_buffer_w*dst_buffer_h+dst_u_of , phy_tgt+dst_buffer_w*dst_buffer_h+dst_buffer_w*dst_buffer_h/4+dst_v_off);
			}else{
				ret = tcc_scaler_yuv420(src_frame_w,src_frame_h,src_image_w, src_image_h, dst_image_w, dst_image_h,dst_frame_w,dst_frame_h,
					phy_src+src_y_off, phy_src+src_buffer_w*src_buffer_h+src_u_off,phy_src+src_buffer_w*src_buffer_h+src_buffer_w*src_buffer_h/4+src_v_off, 
					phy_tgt+dst_y_off, phy_tgt+dst_buffer_w*dst_buffer_h+dst_u_of , phy_tgt+dst_buffer_w*dst_buffer_h+dst_buffer_w*dst_buffer_h/4+dst_v_off,
					s_x_off,s_y_off,d_x_off,d_y_off);
			}
			break;

		case IMAGE_FORMAT_YUV422Sep:
			ret = tcc_scaler_yuv422(src_frame_w,src_frame_h,src_image_w, src_image_h, dst_image_w, dst_image_h,dst_frame_w,dst_frame_h,
					phy_src+src_y_off, phy_src+src_buffer_w*src_buffer_h+src_u_off,phy_src+src_buffer_w*src_buffer_h+src_buffer_w*src_buffer_h/2+src_v_off,  
					phy_tgt+dst_y_off,phy_tgt+dst_buffer_w*dst_buffer_h+dst_u_of,phy_tgt+dst_buffer_w*dst_buffer_h+dst_buffer_w*dst_buffer_h/2+dst_v_off);
			break;

		case IMAGE_FORMAT_RGB565:
			ret = tcc_scaler_rgb565(src_frame_w,src_frame_h,src_image_w, src_image_h,dst_image_w, dst_image_h,dst_frame_w,dst_frame_h,
							phy_src	, 0, 0,phy_tgt, 0, 0);
			break;

		default:
			break;
	}

	return ret;	
}	

/*****************************************************************************
* Function Name : tcc_scaler_yuv422
******************************************************************************
* Desription  : 
* Parameter   :
* Return      :
******************************************************************************/
int tcc_scaler_yuv422( unsigned int s_hsize, unsigned int s_vsize, unsigned int so_hsize,unsigned int so_vsize,
				       unsigned int m_hsize, unsigned int m_vsize,unsigned int do_hsize,unsigned int do_vsize,
                       unsigned int s_y, unsigned int s_u, unsigned int s_v,
                       unsigned int d_y, unsigned int d_u, unsigned int d_v) 
{
	int ret = SCALER_MEMORY_NO_ERROR;
	FILE *mM2m_fd;
	SCALER_TYPE ScaleInfo;
	struct pollfd poll_event[1];

	DEBUG (DEB_LEV_SIMPLE_SEQ,"tcc_scaler_rgb565(%d,%d,%d,%d,%d,%d,%d,%d)\n",s_hsize,s_vsize,so_hsize,so_vsize,m_hsize,m_vsize,do_hsize,do_vsize);
	DEBUG (DEB_LEV_SIMPLE_SEQ,"(0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,%d)\n",s_y,s_u,s_v,d_y,d_u,d_v);	


	mM2m_fd = open( TCC_SCALER_DEV0_NAME, O_RDWR | O_NDELAY);
	if (mM2m_fd <= 0)
	{
		DEBUG (DEB_LEV_TCC_ERR,"can't open '%s'", TCC_SCALER_DEV1_NAME);
		return 0;
	}
	else
		DEBUG (DEB_LEV_TCC_ERR,"m2m sclaer open ok\n");

	ScaleInfo.src_Yaddr			= (char*)s_y;
	ScaleInfo.src_Uaddr			= (char*)s_u;
	ScaleInfo.src_Vaddr			= (char*)s_v;
	ScaleInfo.responsetype 		= SCALER_INTERRUPT; 
	ScaleInfo.src_fmt			= SCALER_YUV422_sp; 	
	ScaleInfo.src_ImgWidth 		= so_hsize;		//	dec out width
	ScaleInfo.src_ImgHeight		= so_vsize;		//	dec out height

	if((s_hsize == so_hsize) && (s_vsize == so_vsize))
	{
		ScaleInfo.src_winLeft		= 0;
		ScaleInfo.src_winTop		= 0;
		ScaleInfo.src_winRight 		= so_hsize;
		ScaleInfo.src_winBottom		= so_vsize;
	}
	else
	{
		if( (so_hsize - s_hsize) != 0)
			ScaleInfo.src_winLeft = ((so_hsize - s_hsize)>>1);
		else
			ScaleInfo.src_winLeft		= 0;
		
		if( (do_vsize - m_vsize) != 0)
			ScaleInfo.src_winTop = (so_vsize - s_vsize)>>1;
		else
			ScaleInfo.src_winTop		= 0;

		ScaleInfo.src_winRight		= so_hsize - ScaleInfo.src_winLeft;
		ScaleInfo.src_winBottom		= so_vsize - ScaleInfo.src_winTop;
	}

	ScaleInfo.dest_Yaddr		= (char*)d_y;
	ScaleInfo.dest_Uaddr		= (char*)d_u;
	ScaleInfo.dest_Vaddr		= (char*)d_v;

	ScaleInfo.dest_fmt 			= SCALER_YUV422_sp; 	
	
	ScaleInfo.dest_ImgWidth		= do_hsize;	//	scaler out width
	ScaleInfo.dest_ImgHeight	= do_vsize;	//	scaler out height

#if 0
	if( (m_hsize == do_hsize) && (m_vsize == do_vsize))
	{
		ScaleInfo.dest_winLeft = 0;
		ScaleInfo.dest_winTop = 0;

		ScaleInfo.dest_winRight 	= do_hsize;
		ScaleInfo.dest_winBottom 	= do_vsize;
	}
	else
	{
		if((do_hsize - m_hsize) != 0)
			ScaleInfo.dest_winLeft = ((do_hsize - m_hsize)>>1);
		else
			ScaleInfo.dest_winLeft = 0;
		
		if( (do_vsize - m_vsize) != 0)
			ScaleInfo.dest_winTop = (do_vsize - m_vsize)>>1;
		else
			ScaleInfo.dest_winTop = 0;

		ScaleInfo.dest_winRight		= do_hsize - ScaleInfo.dest_winLeft;
		ScaleInfo.dest_winBottom	= do_vsize - ScaleInfo.dest_winTop;
	}
#endif

	ScaleInfo.dest_ImgWidth		= do_hsize;	//	scaler out width
	ScaleInfo.dest_ImgHeight	= do_vsize;	//	scaler out height

	ScaleInfo.dest_winLeft = 0;
	ScaleInfo.dest_winTop = 0;
	ScaleInfo.dest_winRight  = do_hsize;
	ScaleInfo.dest_winBottom = do_vsize;

	if ( ioctl( mM2m_fd, TCC_SCALER_IOCTRL, &ScaleInfo) != 0)
	{
		DEBUG (DEB_LEV_TCC_ERR,"Scaler Out Error!" );
		close(mM2m_fd);
		return NULL;
	}
	if(ScaleInfo.responsetype == SCALER_INTERRUPT)
	{
		int ret;

		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd = mM2m_fd;
		poll_event[0].events = POLLIN;
		ret = poll((struct pollfd*)poll_event, 1, 400);

		if (ret < 0) 
		{
			DEBUG (DEB_LEV_TCC_ERR,"m2m poll error\n");
			close(mM2m_fd);
			return NULL;
		}
		else if (ret == 0) 
		{
			DEBUG (DEB_LEV_TCC_ERR,"m2m poll timeout\n");	
			close(mM2m_fd);
			return NULL;
		}
		else if (ret > 0) 
		{
			if (poll_event[0].revents & POLLERR) 
			{
				DEBUG (DEB_LEV_TCC_ERR,"m2m poll POLLERR\n");
				close(mM2m_fd);
				return NULL;
			}
		}

		close(mM2m_fd);
	}

	return ret;
}




/*****************************************************************************
* Function Name : tcc_scaler_rgb565
******************************************************************************
* Desription  : 
* Parameter   :
* Return      :
******************************************************************************/
int tcc_scaler_rgb565( unsigned int s_hsize, unsigned int s_vsize, unsigned int so_hsize,unsigned int so_vsize,
				       unsigned int m_hsize, unsigned int m_vsize,unsigned int do_hsize,unsigned int do_vsize,
                       unsigned int s_y, unsigned int s_u, unsigned int s_v,
                       unsigned int d_y, unsigned int d_u, unsigned int d_v) 
{
	int ret = SCALER_MEMORY_NO_ERROR;
	FILE *mM2m_fd;
	SCALER_TYPE ScaleInfo;
	struct pollfd poll_event[1];

	DEBUG (DEB_LEV_SIMPLE_SEQ,"tcc_scaler_rgb565(%d,%d,%d,%d,%d,%d,%d,%d)\n",s_hsize,s_vsize,so_hsize,so_vsize,m_hsize,m_vsize,do_hsize,do_vsize);
	DEBUG (DEB_LEV_SIMPLE_SEQ,"(0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,%d)\n",s_y,s_u,s_v,d_y,d_u,d_v);	


	mM2m_fd = open( TCC_SCALER_DEV0_NAME, O_RDWR | O_NDELAY);
	if (mM2m_fd <= 0)
	{
		DEBUG (DEB_LEV_TCC_ERR,"can't open'%s'", TCC_SCALER_DEV1_NAME);
		return 0;
	}
	else
		DEBUG (DEB_LEV_TCC_ERR,"m2m sclaer open ok\n");

	ScaleInfo.src_Yaddr			= (char*)s_y;
	ScaleInfo.src_Uaddr			= (char*)s_u;
	ScaleInfo.src_Vaddr			= (char*)s_v;
	ScaleInfo.responsetype 		= SCALER_INTERRUPT; 
	ScaleInfo.src_fmt			= SCALER_RGB565; 	
	ScaleInfo.src_ImgWidth 		= so_hsize;		//	dec out width
	ScaleInfo.src_ImgHeight		= so_vsize;		//	dec out height

	if((s_hsize == so_hsize) && (s_vsize == so_vsize))
	{
		ScaleInfo.src_winLeft		= 0;
		ScaleInfo.src_winTop		= 0;
		ScaleInfo.src_winRight 		= so_hsize;
		ScaleInfo.src_winBottom		= so_vsize;
	}
	else
	{
		if( (so_hsize - s_hsize) != 0)
			ScaleInfo.src_winLeft = ((so_hsize - s_hsize)>>1);
		else
			ScaleInfo.src_winLeft		= 0;
		
		if( (do_vsize - m_vsize) != 0)
			ScaleInfo.src_winTop = (so_vsize - s_vsize)>>1;
		else
			ScaleInfo.src_winTop		= 0;

		ScaleInfo.src_winRight		= so_hsize - ScaleInfo.src_winLeft;
		ScaleInfo.src_winBottom		= so_vsize - ScaleInfo.src_winTop;
	}

	ScaleInfo.dest_Yaddr		= (char*)d_y;
	ScaleInfo.dest_Uaddr		= (char*)d_u;
	ScaleInfo.dest_Vaddr		= (char*)d_v;

	ScaleInfo.dest_fmt 			= SCALER_RGB565; 	
	
	ScaleInfo.dest_ImgWidth		= do_hsize;	//	scaler out width
	ScaleInfo.dest_ImgHeight	= do_vsize;	//	scaler out height

#if 0
	if( (m_hsize == do_hsize) && (m_vsize == do_vsize))
	{
		ScaleInfo.dest_winLeft = 0;
		ScaleInfo.dest_winTop = 0;

		ScaleInfo.dest_winRight 	= do_hsize;
		ScaleInfo.dest_winBottom 	= do_vsize;
	}
	else
	{
		if((do_hsize - m_hsize) != 0)
			ScaleInfo.dest_winLeft = ((do_hsize - m_hsize)>>1);
		else
			ScaleInfo.dest_winLeft = 0;
		
		if( (do_vsize - m_vsize) != 0)
			ScaleInfo.dest_winTop = (do_vsize - m_vsize)>>1;
		else
			ScaleInfo.dest_winTop = 0;

		ScaleInfo.dest_winRight		= do_hsize - ScaleInfo.dest_winLeft;
		ScaleInfo.dest_winBottom	= do_vsize - ScaleInfo.dest_winTop;
	}
#endif

	ScaleInfo.dest_ImgWidth		= do_hsize;	//	scaler out width
	ScaleInfo.dest_ImgHeight	= do_vsize;	//	scaler out height

	ScaleInfo.dest_winLeft = 0;
	ScaleInfo.dest_winTop = 0;
	ScaleInfo.dest_winRight  = do_hsize;
	ScaleInfo.dest_winBottom = do_vsize;

	if ( ioctl( mM2m_fd, TCC_SCALER_IOCTRL, &ScaleInfo) != 0)
	{
		DEBUG (DEB_LEV_TCC_ERR,"Scaler Out Error!" );
		close(mM2m_fd);
		return NULL;
	}
	if(ScaleInfo.responsetype == SCALER_INTERRUPT)
	{
		int ret;

		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd = mM2m_fd;
		poll_event[0].events = POLLIN;
		ret = poll((struct pollfd*)poll_event, 1, 400);

		if (ret < 0) 
		{
			DEBUG (DEB_LEV_TCC_ERR,"m2m poll error\n");
			close(mM2m_fd);
			return NULL;
		}
		else if (ret == 0) 
		{
			DEBUG (DEB_LEV_TCC_ERR,"m2m poll timeout\n");	
			close(mM2m_fd);
			return NULL;
		}
		else if (ret > 0) 
		{
			if (poll_event[0].revents & POLLERR) 
			{
				DEBUG (DEB_LEV_TCC_ERR,"m2m poll POLLERR\n");
				close(mM2m_fd);
				return NULL;
			}
		}

		close(mM2m_fd);
	}

	return ret;
}


/*****************************************************************************
* Function Name : tcc_scaler_execute
******************************************************************************
* Desription  : 
* Parameter   :
* Return      :
******************************************************************************/
int tcc_scaler_yuv422seq( unsigned int s_hsize, unsigned int s_vsize, unsigned int so_hsize,unsigned int so_vsize,
				       unsigned int m_hsize, unsigned int m_vsize,unsigned int do_hsize,unsigned int do_vsize,
                       unsigned int s_y, unsigned int s_u, unsigned int s_v,
                       unsigned int d_y, unsigned int d_u, unsigned int d_v,
                       unsigned int s_x_off,unsigned int s_y_off,unsigned int d_x_off, unsigned int d_y_off,int src_format,int dst_format,int full) 
{
	int ret = SCALER_MEMORY_NO_ERROR;
	FILE *mM2m_fd;
	SCALER_TYPE ScaleInfo;
	struct pollfd poll_event[1];

	DEBUG (DEB_LEV_SIMPLE_SEQ,"tcc_scaler_yuv420(%d,%d,%d,%d,%d,%d,%d,%d)\n",s_hsize,s_vsize,so_hsize,so_vsize,m_hsize,m_vsize,do_hsize,do_vsize);
	DEBUG (DEB_LEV_SIMPLE_SEQ,"(0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,%d)\n",s_y,s_u,s_v,d_y,d_u,d_v);	
	DEBUG (DEB_LEV_SIMPLE_SEQ,"(0x%d,0x%d,0x%d,0x%d)\n",s_x_off,s_y_off,d_x_off,d_y_off);	


	mM2m_fd = open( TCC_SCALER_DEV0_NAME, O_RDWR | O_NDELAY);
	if (mM2m_fd <= 0)
	{
		DEBUG (DEB_LEV_TCC_ERR,"can't open'%s'", TCC_SCALER_DEV1_NAME);
		return 0;
	}
	else
		DEBUG (DEB_LEV_TCC_ERR,"m2m sclaer open ok\n");

	ScaleInfo.src_Yaddr			= (char*)s_y;
	ScaleInfo.src_Uaddr			= (char*)s_u;
	ScaleInfo.src_Vaddr			= (char*)s_v;
	ScaleInfo.responsetype 		= SCALER_INTERRUPT; 
	ScaleInfo.src_fmt			= src_format;	
	ScaleInfo.src_ImgWidth 		= so_hsize;		//	dec out width
	ScaleInfo.src_ImgHeight		= so_vsize;		//	dec out height

	if(full)
	{
		if((s_hsize == so_hsize) && (s_vsize == so_vsize))
		{
			ScaleInfo.src_winLeft		= 0;
			ScaleInfo.src_winTop		= 0;
			ScaleInfo.src_winRight 		= so_hsize;
			ScaleInfo.src_winBottom		= so_vsize;
		}
		else
		{
			if( (so_hsize - s_hsize) != 0)
				ScaleInfo.src_winLeft = ((so_hsize - s_hsize)>>1);
			else
				ScaleInfo.src_winLeft		= 0;
			
			if( (do_vsize - m_vsize) != 0)
				ScaleInfo.src_winTop = (so_vsize - s_vsize)>>1;
			else
				ScaleInfo.src_winTop		= 0;

			ScaleInfo.src_winRight		= so_hsize - ScaleInfo.src_winLeft;
			ScaleInfo.src_winBottom		= so_vsize - ScaleInfo.src_winTop;
		}
	}else{
		if((s_hsize == so_hsize) && (s_vsize == so_vsize))
		{
			ScaleInfo.src_winLeft		= 0;
			ScaleInfo.src_winTop		= 0;
			ScaleInfo.src_winRight 		= so_hsize;
			ScaleInfo.src_winBottom		= so_vsize;
		}
		else
		{
			ScaleInfo.src_winLeft = s_x_off;
			ScaleInfo.src_winTop = s_y_off;
			ScaleInfo.src_winRight		= ScaleInfo.src_winLeft+s_hsize;
			ScaleInfo.src_winBottom		= ScaleInfo.src_winTop+s_vsize;
		}
	}

	ScaleInfo.dest_Yaddr		= (char*)d_y;
	ScaleInfo.dest_Uaddr		= (char*)d_u;
	ScaleInfo.dest_Vaddr		= (char*)d_v;

	ScaleInfo.dest_fmt 			= dst_format;
	
	ScaleInfo.dest_ImgWidth		= do_hsize;	//	scaler out width
	ScaleInfo.dest_ImgHeight	= do_vsize;	//	scaler out height

	if(full)
	{
		ScaleInfo.dest_winLeft = 0;
		ScaleInfo.dest_winTop = 0;
		ScaleInfo.dest_winRight  = do_hsize;
		ScaleInfo.dest_winBottom = do_vsize;
	}else{
		if((m_hsize == do_hsize) && (m_vsize == do_vsize))
		{
			ScaleInfo.dest_winLeft = 0;
			ScaleInfo.dest_winTop = 0;

			ScaleInfo.dest_winRight 	= do_hsize;
			ScaleInfo.dest_winBottom 	= do_vsize;
		}
		else
		{
			ScaleInfo.dest_winLeft = d_x_off;
			ScaleInfo.dest_winTop = d_y_off;
			ScaleInfo.dest_winRight		= ScaleInfo.dest_winLeft + m_hsize;
			ScaleInfo.dest_winBottom	= ScaleInfo.dest_winTop + m_vsize;
		}
	}

	ScaleInfo.dest_ImgWidth		= do_hsize;	//	scaler out width
	ScaleInfo.dest_ImgHeight	= do_vsize;	//	scaler out height

	DEBUG (DEB_LEV_SIMPLE_SEQ,"ScaleInfo.src_winLeft %d ScaleInfo.src_winTop %d\n",ScaleInfo.src_winLeft,ScaleInfo.src_winTop);	
	DEBUG (DEB_LEV_SIMPLE_SEQ,"ScaleInfo.src_winRight %d ScaleInfo.src_winBottom %d\n",ScaleInfo.src_winRight,ScaleInfo.src_winBottom);	


	DEBUG (DEB_LEV_SIMPLE_SEQ,"ScaleInfo.dest_winLeft %d ScaleInfo.dest_winTop %d\n",ScaleInfo.dest_winLeft,ScaleInfo.dest_winTop);	
	DEBUG (DEB_LEV_SIMPLE_SEQ,"ScaleInfo.dest_winRight %d ScaleInfo.dest_winBottom %d\n",ScaleInfo.dest_winRight,ScaleInfo.dest_winBottom);	



	if ( ioctl( mM2m_fd, TCC_SCALER_IOCTRL, &ScaleInfo) != 0)
	{
		DEBUG (DEB_LEV_TCC_ERR,"Scaler Out Error!" );
		close(mM2m_fd);
		return NULL;
	}
	if(ScaleInfo.responsetype == SCALER_INTERRUPT)
	{
		int ret;

		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd = mM2m_fd;
		poll_event[0].events = POLLIN;
		ret = poll((struct pollfd*)poll_event, 1, 3000);

		if (ret < 0) 
		{
			DEBUG (DEB_LEV_TCC_ERR,"m2m poll error\n");
			close(mM2m_fd);
			return NULL;
		}
		else if (ret == 0) 
		{
			DEBUG (DEB_LEV_TCC_ERR,"m2m poll timeout\n");	
			close(mM2m_fd);
			return NULL;
		}
		else if (ret > 0) 
		{
			if (poll_event[0].revents & POLLERR) 
			{
				DEBUG (DEB_LEV_TCC_ERR,"m2m poll POLLERR\n");
				close(mM2m_fd);
				return NULL;
			}
		}

		close(mM2m_fd);
	}

	return ret;
}




/*****************************************************************************
* Function Name : tcc_scaler_yuv420
******************************************************************************
* Desription  : 
* Parameter   :
* Return      :
******************************************************************************/
int tcc_scaler_yuv420( unsigned int s_hsize, unsigned int s_vsize, unsigned int so_hsize,unsigned int so_vsize,
				       unsigned int m_hsize, unsigned int m_vsize,unsigned int do_hsize,unsigned int do_vsize,
                       unsigned int s_y, unsigned int s_u, unsigned int s_v,
                       unsigned int d_y, unsigned int d_u, unsigned int d_v,
                       unsigned int s_x_off,unsigned int s_y_off,unsigned int d_x_off, unsigned int d_y_off) 
{
	int ret = SCALER_MEMORY_NO_ERROR;
	FILE *mM2m_fd;
	SCALER_TYPE ScaleInfo;
	struct pollfd poll_event[1];

	DEBUG (DEB_LEV_SIMPLE_SEQ,"tcc_scaler_yuv420(%d,%d,%d,%d,%d,%d,%d,%d)\n",s_hsize,s_vsize,so_hsize,so_vsize,m_hsize,m_vsize,do_hsize,do_vsize);
	DEBUG (DEB_LEV_SIMPLE_SEQ,"(0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,%d)\n",s_y,s_u,s_v,d_y,d_u,d_v);	
	DEBUG (DEB_LEV_SIMPLE_SEQ,"(0x%d,0x%d,0x%d,0x%d)\n",s_x_off,s_y_off,d_x_off,d_y_off);	


	mM2m_fd = open( TCC_SCALER_DEV0_NAME, O_RDWR | O_NDELAY);
	if (mM2m_fd <= 0)
	{
		DEBUG (DEB_LEV_TCC_ERR,"can't open'%s'", TCC_SCALER_DEV1_NAME);
		return 0;
	}
	else
		DEBUG (DEB_LEV_TCC_ERR,"m2m sclaer open ok\n");

	ScaleInfo.src_Yaddr			= (char*)s_y;
	ScaleInfo.src_Uaddr			= (char*)s_u;
	ScaleInfo.src_Vaddr			= (char*)s_v;
	ScaleInfo.responsetype 		= SCALER_INTERRUPT; 
	ScaleInfo.src_fmt			= SCALER_YUV420_sp; 	
	ScaleInfo.src_ImgWidth 		= so_hsize;		//	dec out width
	ScaleInfo.src_ImgHeight		= so_vsize;		//	dec out height

	if((s_hsize == so_hsize) && (s_vsize == so_vsize))
	{
		ScaleInfo.src_winLeft		= 0;
		ScaleInfo.src_winTop		= 0;
		ScaleInfo.src_winRight 		= so_hsize;
		ScaleInfo.src_winBottom		= so_vsize;
	}
	else
	{
	#if 0
		if( (so_hsize - s_hsize) != 0)
			ScaleInfo.src_winLeft = ((so_hsize - s_hsize)>>1);
		else
			ScaleInfo.src_winLeft		= 0;
		
		if( (do_vsize - m_vsize) != 0)
			ScaleInfo.src_winTop = (so_vsize - s_vsize)>>1;
		else
			ScaleInfo.src_winTop		= 0;
	#else
		ScaleInfo.src_winLeft = s_x_off;
		ScaleInfo.src_winTop = s_y_off;
	#endif
		ScaleInfo.src_winRight		= ScaleInfo.src_winLeft+s_hsize;
		ScaleInfo.src_winBottom		= ScaleInfo.src_winTop+s_vsize;
	}

	ScaleInfo.dest_Yaddr		= (char*)d_y;
	ScaleInfo.dest_Uaddr		= (char*)d_u;
	ScaleInfo.dest_Vaddr		= (char*)d_v;

	ScaleInfo.dest_fmt 			= SCALER_YUV420_sp; 	
	
	ScaleInfo.dest_ImgWidth		= do_hsize;	//	scaler out width
	ScaleInfo.dest_ImgHeight	= do_vsize;	//	scaler out height

	if( (m_hsize == do_hsize) && (m_vsize == do_vsize))
	{
		ScaleInfo.dest_winLeft = 0;
		ScaleInfo.dest_winTop = 0;

		ScaleInfo.dest_winRight 	= do_hsize;
		ScaleInfo.dest_winBottom 	= do_vsize;
	}
	else
	{
	#if 0
		if((do_hsize - m_hsize) != 0)
			ScaleInfo.dest_winLeft = ((do_hsize - m_hsize)>>1);
		else
			ScaleInfo.dest_winLeft = 0;
		
		if( (do_vsize - m_vsize) != 0)
			ScaleInfo.dest_winTop = (do_vsize - m_vsize)>>1;
		else
			ScaleInfo.dest_winTop = 0;
	#else

		ScaleInfo.dest_winLeft = d_x_off;
		ScaleInfo.dest_winTop = d_y_off;
	#endif	
		ScaleInfo.dest_winRight		= ScaleInfo.dest_winLeft + m_hsize;
		ScaleInfo.dest_winBottom	= ScaleInfo.dest_winTop + m_vsize;
	}

	ScaleInfo.dest_ImgWidth		= do_hsize;	//	scaler out width
	ScaleInfo.dest_ImgHeight	= do_vsize;	//	scaler out height

	DEBUG (DEB_LEV_SIMPLE_SEQ,"ScaleInfo.src_winLeft %d ScaleInfo.src_winTop %d\n",ScaleInfo.src_winLeft,ScaleInfo.src_winTop);	
	DEBUG (DEB_LEV_SIMPLE_SEQ,"ScaleInfo.src_winRight %d ScaleInfo.src_winBottom %d\n",ScaleInfo.src_winRight,ScaleInfo.src_winBottom);	


	DEBUG (DEB_LEV_SIMPLE_SEQ,"ScaleInfo.dest_winLeft %d ScaleInfo.dest_winTop %d\n",ScaleInfo.dest_winLeft,ScaleInfo.dest_winTop);	
	DEBUG (DEB_LEV_SIMPLE_SEQ,"ScaleInfo.dest_winRight %d ScaleInfo.dest_winBottom %d\n",ScaleInfo.dest_winRight,ScaleInfo.dest_winBottom);	



	if ( ioctl( mM2m_fd, TCC_SCALER_IOCTRL, &ScaleInfo) != 0)
	{
		DEBUG (DEB_LEV_TCC_ERR,"Scaler Out Error!" );
		close(mM2m_fd);
		return NULL;
	}
	if(ScaleInfo.responsetype == SCALER_INTERRUPT)
	{
		int ret;

		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd = mM2m_fd;
		poll_event[0].events = POLLIN;
		ret = poll((struct pollfd*)poll_event, 1, 3000);

		if (ret < 0) 
		{
			DEBUG (DEB_LEV_TCC_ERR,"m2m poll error\n");
			close(mM2m_fd);
			return NULL;
		}
		else if (ret == 0) 
		{
			DEBUG (DEB_LEV_TCC_ERR,"m2m poll timeout\n");	
			close(mM2m_fd);
			return NULL;
		}
		else if (ret > 0) 
		{
			if (poll_event[0].revents & POLLERR) 
			{
				DEBUG (DEB_LEV_TCC_ERR,"m2m poll POLLERR\n");
				close(mM2m_fd);
				return NULL;
			}
		}

		close(mM2m_fd);
	}

	return ret;
}



/*****************************************************************************
* Function Name : tcc_scaler_yuv420_full
******************************************************************************
* Desription  : 
* Parameter   :
* Return      :
******************************************************************************/
int tcc_scaler_yuv420_full( unsigned int s_hsize, unsigned int s_vsize, unsigned int so_hsize,unsigned int so_vsize,
					    unsigned int m_hsize, unsigned int m_vsize,unsigned int do_hsize,unsigned int do_vsize,
	                    unsigned int s_y, unsigned int s_u, unsigned int s_v,
	                    unsigned int d_y, unsigned int d_u, unsigned int d_v)
{
	int ret = SCALER_MEMORY_NO_ERROR;
	FILE *mM2m_fd;
	SCALER_TYPE ScaleInfo;
	struct pollfd poll_event[1];

	DEBUG (DEB_LEV_SIMPLE_SEQ,"tcc_scaler_yuv420_full(%d,%d,%d,%d,%d,%d,%d,%d)\n",s_hsize,s_vsize,so_hsize,so_vsize,m_hsize,m_vsize,do_hsize,do_vsize);
	DEBUG (DEB_LEV_SIMPLE_SEQ,"(0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",s_y,s_u,s_v,d_y,d_u,d_v);	

	mM2m_fd = open( TCC_SCALER_DEV0_NAME, O_RDWR | O_NDELAY);
	if (mM2m_fd <= 0)
	{
		DEBUG (DEB_LEV_TCC_ERR,"can't open'%s'",  TCC_SCALER_DEV0_NAME);
		return 0;
	}
	else
	{
	//	DEBUG (DEB_LEV_TCC_ERR,"m2m sclaer open ok\n");
	}


	filt2d_test(mM2m_fd);

	
	ScaleInfo.src_Yaddr			= (char*)s_y;
	ScaleInfo.src_Uaddr			= (char*)s_u;
	ScaleInfo.src_Vaddr			= (char*)s_v;
	ScaleInfo.responsetype 		= SCALER_INTERRUPT; 
	ScaleInfo.src_fmt			= 24;//SCALER_YUV420_sp; 	
	ScaleInfo.src_ImgWidth 		= so_hsize;		//	dec out width
	ScaleInfo.src_ImgHeight		= so_vsize;		//	dec out height

	if((s_hsize == so_hsize) && (s_vsize == so_vsize))
	{
		ScaleInfo.src_winLeft		= 0;
		ScaleInfo.src_winTop		= 0;
		ScaleInfo.src_winRight 		= so_hsize;
		ScaleInfo.src_winBottom		= so_vsize;
	}
	else
	{
		if( (so_hsize - s_hsize) != 0)
			ScaleInfo.src_winLeft = ((so_hsize - s_hsize)>>1);
		else
			ScaleInfo.src_winLeft		= 0;
		
		if( (do_vsize - m_vsize) != 0)
			ScaleInfo.src_winTop = (so_vsize - s_vsize)>>1;
		else
			ScaleInfo.src_winTop		= 0;

		ScaleInfo.src_winRight		= so_hsize - ScaleInfo.src_winLeft;
		ScaleInfo.src_winBottom		= so_vsize - ScaleInfo.src_winTop;
	}
	
	ScaleInfo.dest_Yaddr		= (char*)d_y;
	ScaleInfo.dest_Uaddr		= (char*)d_u;
	ScaleInfo.dest_Vaddr		= (char*)d_v;

	ScaleInfo.dest_fmt 			= 24;//SCALER_YUV420_sp; 	
	
	ScaleInfo.dest_ImgWidth		= do_hsize;	//	scaler out width
	ScaleInfo.dest_ImgHeight	= do_vsize;	//	scaler out height

	ScaleInfo.dest_winLeft = 0;
	ScaleInfo.dest_winTop = 0;
	ScaleInfo.dest_winRight  = do_hsize;
	ScaleInfo.dest_winBottom = do_vsize;

	if ( ioctl( mM2m_fd, TCC_SCALER_IOCTRL, &ScaleInfo) != 0)
	{
		DEBUG (DEB_LEV_TCC_ERR,"Scaler Out Error!" );
		close(mM2m_fd);
		return NULL;
	}
	if(ScaleInfo.responsetype == SCALER_INTERRUPT)
	{
		int ret;

		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd = mM2m_fd;
		poll_event[0].events = POLLIN;
		ret = poll((struct pollfd*)poll_event, 1, 400);

		if (ret < 0) 
		{
			DEBUG (DEB_LEV_TCC_ERR,"m2m poll error\n");
			close(mM2m_fd);
			return NULL;
		}
		else if (ret == 0) 
		{
			DEBUG (DEB_LEV_TCC_ERR,"m2m poll timeout\n");	
			close(mM2m_fd);
			return NULL;
		}
		else if (ret > 0) 
		{
			if (poll_event[0].revents & POLLERR) 
			{
				DEBUG (DEB_LEV_TCC_ERR,"m2m poll POLLERR\n");
				close(mM2m_fd);
				return NULL;
			}
		}

		close(mM2m_fd);
	}

	return ret;
}

/*
#############################################################################################
							SCALER INTERFACE FOR CAMERA
#############################################################################################
*/

/*****************************************************************************
* Function Name : 
******************************************************************************
* Desription  : tcc_camera_scaler_open
* Parameter   : dev_name:device driver name 
* Return	  : error:-1, others file descriptor
******************************************************************************/
int tcc_camera_scaler_open(const char* dev_name)
{
	int fd = -1;

	if(dev_name != NULL)
	{
		fd = open(dev_name, O_RDWR | O_NDELAY);
		if(fd >= 0)
		{
			DEBUG (DEB_LEV_TCC,"%s open succeed\n", dev_name);
		}
		else
			DEBUG (DEB_LEV_ERR, "can't open %s\n", dev_name);
	}

	return fd;
}

/*****************************************************************************
* Function Name : 
******************************************************************************
* Desription  : tcc_camera_scaler_execute
* Parameter   : fd:file descriptor

				src: addr_y, addr_u, addr_v is src address 
					 x,y,w,h is input image size
					 w_x, w_y, w_w, w_h is window size (it can use for zoom-in)

				dst: 
					addr_y, addr_u, addr_v is dest address 
					x,y,w,h is destination size
				
* Return	  : error:-1, others succeed
******************************************************************************/
int tcc_camera_scaler_execute(const int fd, tcc_scaler_info_t* src, tcc_scaler_info_t* dst)
{
#if 1	

	int ret;
	struct pollfd poll_event[1];
	SCALER_TYPE scaler_info;

	scaler_info.responsetype 		= SCALER_INTERRUPT; 
	
	scaler_info.src_Yaddr			= src->addr_y;
	scaler_info.src_Uaddr			= src->addr_u;
	scaler_info.src_Vaddr			= src->addr_v;

//	scaler_info.src_fmt				= SCALER_YUV420_sp; 	
	scaler_info.src_fmt				= src->format;
	scaler_info.src_ImgWidth 		= src->w;		
	scaler_info.src_ImgHeight		= src->h;	
	
	scaler_info.src_winLeft			= src->x;
	scaler_info.src_winTop			= src->y;
	scaler_info.src_winRight 		= src->w;		
	scaler_info.src_winBottom		= src->h;		

	scaler_info.dest_Yaddr			= dst->addr_y;
	scaler_info.dest_Uaddr			= dst->addr_u;
	scaler_info.dest_Vaddr			= dst->addr_v;
	
//	scaler_info.dest_fmt 			= SCALER_YUV420_sp; 	
	scaler_info.dest_fmt 			= dst->format;
	scaler_info.dest_ImgWidth		= dst->w;
	scaler_info.dest_ImgHeight		= dst->h;
	
	scaler_info.dest_winLeft 		= dst->x;
	scaler_info.dest_winTop			= dst->y;
 	scaler_info.dest_winRight		= dst->w;
	scaler_info.dest_winBottom		= dst->h;

  	scaler_info.viqe_onthefly = 0;
  	scaler_info.interlaced = 0;	
  
	if(ioctl(fd, TCC_SCALER_IOCTRL, &scaler_info) != 0)
	{
		DEBUG (DEB_LEV_ERR, "scaler error\n");
		close(fd);
		return -1;
	}

	if(scaler_info.responsetype == SCALER_INTERRUPT)
	{
		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd = fd;
		poll_event[0].events = POLLIN;
		ret = poll((struct pollfd*)poll_event, 1, 400);

		if (ret < 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if (ret == 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if (ret > 0) 
		{
			if (poll_event[0].revents & POLLERR) 
			{
				DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
				return -1;
			}
		}
	}

	return 0;


#else
	int ret;
	struct pollfd poll_event[1];
	SCALER_TYPE scaler_info;

	if(fd < 0)
		return -1;
		
	scaler_info.responsetype 		= SCALER_INTERRUPT; 
	
	scaler_info.src_Yaddr			= src->addr_y;
	scaler_info.src_Uaddr			= src->addr_u;
	scaler_info.src_Vaddr			= src->addr_v;
	scaler_info.src_fmt				= src->format;
	scaler_info.src_ImgWidth 		= src->w;		
	scaler_info.src_ImgHeight		= src->h;	
	
	scaler_info.src_winLeft			= src->w_x;
	scaler_info.src_winTop			= src->w_y;
	scaler_info.src_winRight 		= src->w_w;		
	scaler_info.src_winBottom		= src->w_h;		

	scaler_info.dest_Yaddr			= dst->addr_y;
	scaler_info.dest_Uaddr			= dst->addr_u;
	scaler_info.dest_Vaddr			= dst->addr_v;
	
	scaler_info.dest_fmt 			= dst->format; 	
	scaler_info.dest_ImgWidth		= dst->w;
	scaler_info.dest_ImgHeight		= dst->h;
	
	scaler_info.dest_winLeft 		= dst->x;
	scaler_info.dest_winTop			= dst->y;
 	scaler_info.dest_winRight		= dst->w;
	scaler_info.dest_winBottom		= dst->h;
	
  //	scaler_info.viqe_onthefly = 0;
  //	scaler_info.interlaced = 0;
  
	if(ioctl(fd, TCC_SCALER_IOCTRL, &scaler_info) != 0)
	{
		DEBUG (DEB_LEV_ERR, "scaler error\n");
		close(fd);
		return -1;
	}

	if(scaler_info.responsetype == SCALER_INTERRUPT)
	{
		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd = fd;
		poll_event[0].events = POLLIN;
		ret = poll((struct pollfd*)poll_event, 1, 400);

		if(ret < 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if(ret == 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if(ret > 0) 
		{
			if (poll_event[0].revents & POLLERR) 
			{
				DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
				return -1;
			}
		}
	}
#endif
}

/*****************************************************************************
* Function Name : 
******************************************************************************
* Desription  : tcc_camera_scaler_execute_2port_vertical
* Parameter   : fd:file descriptor

				src: addr_y, addr_u, addr_v is src address 
					 x,y,w,h is input image size
					 w_x, w_y, w_w, w_h is window size (it can use for zoom-in)

				dst: 
					addr_y, addr_u, addr_v is dest address 
					x,y,w,h is destination size
				
* Return	  : error:-1, others succeed
******************************************************************************/
int tcc_camera_scaler_execute_2port_vertical(const int fd, tcc_scaler_info_t* src, tcc_scaler_info_t* dst, int PortIndex)
{
#if 1	

	int ret;
	struct pollfd poll_event[1];
	SCALER_TYPE scaler_info;

	scaler_info.responsetype 		= SCALER_INTERRUPT; 
	
	scaler_info.src_Yaddr			= src->addr_y;
	scaler_info.src_Uaddr			= src->addr_u;
	scaler_info.src_Vaddr			= src->addr_v;

//	scaler_info.src_fmt				= SCALER_YUV420_sp; 	
	scaler_info.src_fmt				= src->format;
	scaler_info.src_ImgWidth 		= src->w;		
	scaler_info.src_ImgHeight		= src->h;	
	
	scaler_info.src_winLeft			= src->x;
	scaler_info.src_winTop			= src->y;
	scaler_info.src_winRight 		= src->w;		
	scaler_info.src_winBottom		= src->h;		

	scaler_info.dest_Yaddr			= dst->addr_y;
	scaler_info.dest_Uaddr			= dst->addr_u;
	scaler_info.dest_Vaddr			= dst->addr_v;
	
//	scaler_info.dest_fmt 			= SCALER_YUV420_sp; 	
	scaler_info.dest_fmt 			= dst->format;
	scaler_info.dest_ImgWidth		= dst->w;
	scaler_info.dest_ImgHeight		= dst->h;
	
	if (PortIndex == 0)
	{
		scaler_info.dest_winLeft 		= dst->x;
		scaler_info.dest_winTop			= dst->y;
 		scaler_info.dest_winRight		= dst->w/2;
		scaler_info.dest_winBottom		= dst->h;
	}
	else
	{
		scaler_info.dest_winLeft 		= dst->x + dst->w/2;
		scaler_info.dest_winTop			= dst->y;
 		scaler_info.dest_winRight		= dst->w;
		scaler_info.dest_winBottom		= dst->h;
	}

  	scaler_info.viqe_onthefly = 0;
  	scaler_info.interlaced = 0;	
  
	if(ioctl(fd, TCC_SCALER_IOCTRL, &scaler_info) != 0)
	{
		DEBUG (DEB_LEV_ERR, "scaler error\n");
		close(fd);
		return -1;
	}

	if(scaler_info.responsetype == SCALER_INTERRUPT)
	{
		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd = fd;
		poll_event[0].events = POLLIN;
		ret = poll((struct pollfd*)poll_event, 1, 400);

		if (ret < 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if (ret == 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if (ret > 0) 
		{
			if (poll_event[0].revents & POLLERR) 
			{
				DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
				return -1;
			}
		}
	}

	return 0;


#else
	int ret;
	struct pollfd poll_event[1];
	SCALER_TYPE scaler_info;

	if(fd < 0)
		return -1;
		
	scaler_info.responsetype 		= SCALER_INTERRUPT; 
	
	scaler_info.src_Yaddr			= src->addr_y;
	scaler_info.src_Uaddr			= src->addr_u;
	scaler_info.src_Vaddr			= src->addr_v;
	scaler_info.src_fmt				= src->format;
	scaler_info.src_ImgWidth 		= src->w;		
	scaler_info.src_ImgHeight		= src->h;	
	
	scaler_info.src_winLeft			= src->w_x;
	scaler_info.src_winTop			= src->w_y;
	scaler_info.src_winRight 		= src->w_w;		
	scaler_info.src_winBottom		= src->w_h;		

	scaler_info.dest_Yaddr			= dst->addr_y;
	scaler_info.dest_Uaddr			= dst->addr_u;
	scaler_info.dest_Vaddr			= dst->addr_v;
	
	scaler_info.dest_fmt 			= dst->format; 	
	scaler_info.dest_ImgWidth		= dst->w;
	scaler_info.dest_ImgHeight		= dst->h;
	
	scaler_info.dest_winLeft 		= dst->x;
	scaler_info.dest_winTop			= dst->y;
 	scaler_info.dest_winRight		= dst->w;
	scaler_info.dest_winBottom		= dst->h;
	
  //	scaler_info.viqe_onthefly = 0;
  //	scaler_info.interlaced = 0;
  
	if(ioctl(fd, TCC_SCALER_IOCTRL, &scaler_info) != 0)
	{
		DEBUG (DEB_LEV_ERR, "scaler error\n");
		close(fd);
		return -1;
	}

	if(scaler_info.responsetype == SCALER_INTERRUPT)
	{
		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd = fd;
		poll_event[0].events = POLLIN;
		ret = poll((struct pollfd*)poll_event, 1, 400);

		if(ret < 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if(ret == 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if(ret > 0) 
		{
			if (poll_event[0].revents & POLLERR) 
			{
				DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
				return -1;
			}
		}
	}
#endif
}

/*****************************************************************************
* Function Name : 
******************************************************************************
* Desription  : tcc_camera_scaler_execute_2port_horizontal
* Parameter   : fd:file descriptor

				src: addr_y, addr_u, addr_v is src address 
					 x,y,w,h is input image size
					 w_x, w_y, w_w, w_h is window size (it can use for zoom-in)

				dst: 
					addr_y, addr_u, addr_v is dest address 
					x,y,w,h is destination size
				
* Return	  : error:-1, others succeed
******************************************************************************/
int tcc_camera_scaler_execute_2port_horizontal(const int fd, tcc_scaler_info_t* src, tcc_scaler_info_t* dst, int PortIndex)
{
#if 1	

	int ret;
	struct pollfd poll_event[1];
	SCALER_TYPE scaler_info;

	scaler_info.responsetype 		= SCALER_INTERRUPT; 
	
	scaler_info.src_Yaddr			= src->addr_y;
	scaler_info.src_Uaddr			= src->addr_u;
	scaler_info.src_Vaddr			= src->addr_v;

//	scaler_info.src_fmt				= SCALER_YUV420_sp; 	
	scaler_info.src_fmt				= src->format;
	scaler_info.src_ImgWidth 		= src->w;		
	scaler_info.src_ImgHeight		= src->h;	
	
	scaler_info.src_winLeft			= src->x;
	scaler_info.src_winTop			= src->y;
	scaler_info.src_winRight 		= src->w;		
	scaler_info.src_winBottom		= src->h;		

	scaler_info.dest_Yaddr			= dst->addr_y;
	scaler_info.dest_Uaddr			= dst->addr_u;
	scaler_info.dest_Vaddr			= dst->addr_v;
	
//	scaler_info.dest_fmt 			= SCALER_YUV420_sp; 	
	scaler_info.dest_fmt 			= dst->format;
	scaler_info.dest_ImgWidth		= dst->w;
	scaler_info.dest_ImgHeight		= dst->h;
	
	if (PortIndex == 0)
	{
		scaler_info.dest_winLeft 		= dst->x;
		scaler_info.dest_winTop			= dst->y;
 		scaler_info.dest_winRight		= dst->w;
		scaler_info.dest_winBottom		= dst->h/2;
	}
	else
	{
		scaler_info.dest_winLeft 		= dst->x;
		scaler_info.dest_winTop			= dst->y + dst->h/2;
 		scaler_info.dest_winRight		= dst->w;
		scaler_info.dest_winBottom		= dst->h;
	}

  	scaler_info.viqe_onthefly = 0;
  	scaler_info.interlaced = 0;	
  
	if(ioctl(fd, TCC_SCALER_IOCTRL, &scaler_info) != 0)
	{
		DEBUG (DEB_LEV_ERR, "scaler error\n");
		close(fd);
		return -1;
	}

	if(scaler_info.responsetype == SCALER_INTERRUPT)
	{
		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd = fd;
		poll_event[0].events = POLLIN;
		ret = poll((struct pollfd*)poll_event, 1, 400);

		if (ret < 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if (ret == 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if (ret > 0) 
		{
			if (poll_event[0].revents & POLLERR) 
			{
				DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
				return -1;
			}
		}
	}

	return 0;


#else
	int ret;
	struct pollfd poll_event[1];
	SCALER_TYPE scaler_info;

	if(fd < 0)
		return -1;
		
	scaler_info.responsetype 		= SCALER_INTERRUPT; 
	
	scaler_info.src_Yaddr			= src->addr_y;
	scaler_info.src_Uaddr			= src->addr_u;
	scaler_info.src_Vaddr			= src->addr_v;
	scaler_info.src_fmt				= src->format;
	scaler_info.src_ImgWidth 		= src->w;		
	scaler_info.src_ImgHeight		= src->h;	
	
	scaler_info.src_winLeft			= src->w_x;
	scaler_info.src_winTop			= src->w_y;
	scaler_info.src_winRight 		= src->w_w;		
	scaler_info.src_winBottom		= src->w_h;		

	scaler_info.dest_Yaddr			= dst->addr_y;
	scaler_info.dest_Uaddr			= dst->addr_u;
	scaler_info.dest_Vaddr			= dst->addr_v;
	
	scaler_info.dest_fmt 			= dst->format; 	
	scaler_info.dest_ImgWidth		= dst->w;
	scaler_info.dest_ImgHeight		= dst->h;
	
	scaler_info.dest_winLeft 		= dst->x;
	scaler_info.dest_winTop			= dst->y;
 	scaler_info.dest_winRight		= dst->w;
	scaler_info.dest_winBottom		= dst->h;
	
  //	scaler_info.viqe_onthefly = 0;
  //	scaler_info.interlaced = 0;
  
	if(ioctl(fd, TCC_SCALER_IOCTRL, &scaler_info) != 0)
	{
		DEBUG (DEB_LEV_ERR, "scaler error\n");
		close(fd);
		return -1;
	}

	if(scaler_info.responsetype == SCALER_INTERRUPT)
	{
		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd = fd;
		poll_event[0].events = POLLIN;
		ret = poll((struct pollfd*)poll_event, 1, 400);

		if(ret < 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if(ret == 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if(ret > 0) 
		{
			if (poll_event[0].revents & POLLERR) 
			{
				DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
				return -1;
			}
		}
	}
#endif
}

/*****************************************************************************
* Function Name : 
******************************************************************************
* Desription  : tcc_camera_scaler_execute_4port
* Parameter   : fd:file descriptor

				src: addr_y, addr_u, addr_v is src address 
					 x,y,w,h is input image size
					 w_x, w_y, w_w, w_h is window size (it can use for zoom-in)

				dst: 
					addr_y, addr_u, addr_v is dest address 
					x,y,w,h is destination size
				
* Return	  : error:-1, others succeed
******************************************************************************/
int tcc_camera_scaler_execute_4port(const int fd, tcc_scaler_info_t* src, tcc_scaler_info_t* dst, int PortIndex)
{
#if 1	

	int ret;
	struct pollfd poll_event[1];
	SCALER_TYPE scaler_info;

	scaler_info.responsetype 		= SCALER_INTERRUPT; 
	
	scaler_info.src_Yaddr			= src->addr_y;
	scaler_info.src_Uaddr			= src->addr_u;
	scaler_info.src_Vaddr			= src->addr_v;

//	scaler_info.src_fmt				= SCALER_YUV420_sp; 	
	scaler_info.src_fmt				= src->format;
	scaler_info.src_ImgWidth 		= src->w;		
	scaler_info.src_ImgHeight		= src->h;	
	
	scaler_info.src_winLeft			= src->x;
	scaler_info.src_winTop			= src->y;
	scaler_info.src_winRight 		= src->w;		
	scaler_info.src_winBottom		= src->h;		

	scaler_info.dest_Yaddr			= dst->addr_y;
	scaler_info.dest_Uaddr			= dst->addr_u;
	scaler_info.dest_Vaddr			= dst->addr_v;
	
//	scaler_info.dest_fmt 			= SCALER_YUV420_sp; 	
	scaler_info.dest_fmt 			= dst->format;
	scaler_info.dest_ImgWidth		= dst->w;
	scaler_info.dest_ImgHeight		= dst->h;
	
	if (PortIndex == 0)
	{
		scaler_info.dest_winLeft 		= dst->x;
		scaler_info.dest_winTop			= dst->y;
 		scaler_info.dest_winRight		= dst->w/2;
		scaler_info.dest_winBottom		= dst->h/2;
	}
	else if (PortIndex == 1)
	{
		scaler_info.dest_winLeft 		= dst->x + dst->w/2;
		scaler_info.dest_winTop			= dst->y;
 		scaler_info.dest_winRight		= dst->w;
		scaler_info.dest_winBottom		= dst->h/2;
	}
	else if (PortIndex == 2)
	{
		scaler_info.dest_winLeft 		= dst->x;
		scaler_info.dest_winTop			= dst->y + dst->h/2;
 		scaler_info.dest_winRight		= dst->w/2;
		scaler_info.dest_winBottom		= dst->h;
	}
	else if (PortIndex == 3)
	{
		scaler_info.dest_winLeft 		= dst->x + dst->w/2;
		scaler_info.dest_winTop			= dst->y + dst->h/2;
 		scaler_info.dest_winRight		= dst->w;
		scaler_info.dest_winBottom		= dst->h;
	}

  	scaler_info.viqe_onthefly = 0;
  	scaler_info.interlaced = 0;	
  
	if(ioctl(fd, TCC_SCALER_IOCTRL, &scaler_info) != 0)
	{
		DEBUG (DEB_LEV_ERR, "scaler error\n");
		close(fd);
		return -1;
	}

	if(scaler_info.responsetype == SCALER_INTERRUPT)
	{
		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd = fd;
		poll_event[0].events = POLLIN;
		ret = poll((struct pollfd*)poll_event, 1, 400);

		if (ret < 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if (ret == 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if (ret > 0) 
		{
			if (poll_event[0].revents & POLLERR) 
			{
				DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
				return -1;
			}
		}
	}

	return 0;


#else
	int ret;
	struct pollfd poll_event[1];
	SCALER_TYPE scaler_info;

	if(fd < 0)
		return -1;
		
	scaler_info.responsetype 		= SCALER_INTERRUPT; 
	
	scaler_info.src_Yaddr			= src->addr_y;
	scaler_info.src_Uaddr			= src->addr_u;
	scaler_info.src_Vaddr			= src->addr_v;
	scaler_info.src_fmt				= src->format;
	scaler_info.src_ImgWidth 		= src->w;		
	scaler_info.src_ImgHeight		= src->h;	
	
	scaler_info.src_winLeft			= src->w_x;
	scaler_info.src_winTop			= src->w_y;
	scaler_info.src_winRight 		= src->w_w;		
	scaler_info.src_winBottom		= src->w_h;		

	scaler_info.dest_Yaddr			= dst->addr_y;
	scaler_info.dest_Uaddr			= dst->addr_u;
	scaler_info.dest_Vaddr			= dst->addr_v;
	
	scaler_info.dest_fmt 			= dst->format; 	
	scaler_info.dest_ImgWidth		= dst->w;
	scaler_info.dest_ImgHeight		= dst->h;
	
	scaler_info.dest_winLeft 		= dst->x;
	scaler_info.dest_winTop			= dst->y;
 	scaler_info.dest_winRight		= dst->w;
	scaler_info.dest_winBottom		= dst->h;
	
  //	scaler_info.viqe_onthefly = 0;
  //	scaler_info.interlaced = 0;
  
	if(ioctl(fd, TCC_SCALER_IOCTRL, &scaler_info) != 0)
	{
		DEBUG (DEB_LEV_ERR, "scaler error\n");
		close(fd);
		return -1;
	}

	if(scaler_info.responsetype == SCALER_INTERRUPT)
	{
		memset(poll_event, 0, sizeof(poll_event));
		poll_event[0].fd = fd;
		poll_event[0].events = POLLIN;
		ret = poll((struct pollfd*)poll_event, 1, 400);

		if(ret < 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if(ret == 0) 
		{
			DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
			return -1;
		}
		else if(ret > 0) 
		{
			if (poll_event[0].revents & POLLERR) 
			{
				DEBUG (DEB_LEV_ERR, "m2m poll error %d\n", __LINE__);
				return -1;
			}
		}
	}
#endif
}

#ifdef SCALER_TEST_APP
#include "pmap.h"

#define WIDTH 	1280
#define HEIGHT 	720

int main(int argc, char** argv)
{
	int src_width, src_height;
	int dst_width, dst_height;
	
	unsigned char* scaler_ptr;
	unsigned char* scaler_dst_ptr;
	int phy_addr;
	pmap_t pmap_scaler;
	pmap_t pmap_scaler_dst;
	int memory_fd;
	FILE* input_fp = NULL;
	FILE* output_fp =  NULL;

	int scaler_fd;
	tcc_scaler_info_t src;
	tcc_scaler_info_t dst;
	
	if(argc < 4)
	{
		printf("usage: TCC_892X_scaler_test source.yuv src_width src_height tar_width tar_height\n");
		return -1;
	}

	src_width = atoi(argv[2]);
	src_height = atoi(argv[3]);
	dst_width = atoi(argv[4]);
	dst_height = atoi(argv[5]);
	
	printf("TCC Scaler Test Program!! input:%s, %d x %d, target: %d x %d\n", argv[1], src_width, src_height, dst_width, dst_height);
	
	input_fp = fopen(argv[1], "r");
	if(input_fp == NULL)
	{
		printf("can't open %s\n", argv[1]);
		return -1;
	}

	output_fp = fopen("scaler_out.yuv", "w");
	if(output_fp == NULL)
	{
		printf("can't open output file\n");
		return -1;
	}

	memory_fd = open("/dev/mem", O_RDWR | O_NDELAY);
	if(memory_fd < 0)
	{
		printf("mem open erroe!!\n");
		return -1;
	}

	//get input memory information
	pmap_get_info("fb_scale", &pmap_scaler);
	scaler_ptr = (unsigned char*)mmap(0, pmap_scaler.size, PROT_READ | PROT_WRITE, MAP_SHARED, memory_fd, pmap_scaler.base);
	if(scaler_ptr == MAP_FAILED)
	{
		printf("mmap error!!\n");
		return -1;
	}

	//get output memory information
	pmap_get_info("fb_scale1", &pmap_scaler_dst);
	scaler_dst_ptr = (unsigned char*)mmap(0, pmap_scaler_dst.size, PROT_READ | PROT_WRITE, MAP_SHARED, memory_fd, pmap_scaler_dst.base);
	if(scaler_dst_ptr == MAP_FAILED)
	{
		printf("mmap error!!\n");
		return -1;
	}

	
	//read yuv420
	if(fread(scaler_ptr, 1, src_width * src_height * 3 / 2, input_fp) <= 0)
	{
		printf("read error!!\n");
		return -1;
	}

	//scaler driver open
	scaler_fd = tcc_scaler_open("/dev/scaler");
	if(scaler_fd < 0)
	{
		printf("scaler_fd open error!!\n");
		return -1;
	}

	//source setting
	phy_addr = pmap_scaler.base;
	src.addr_y = GET_ADDR_YUV42X_spY(phy_addr);
	src.addr_u = GET_ADDR_YUV42X_spU(src.addr_y, src_width, src_height);
	src.addr_v = GET_ADDR_YUV420_spV(src.addr_u, src_width, src_height);
	src.x = 0;
	src.y = 0;
	src.w = src_width;
	src.h = src_height;
	src.w_x = 0;
	src.w_y = 0;
	src.w_w = src_width;
	src.w_h = src_height;
	src.format = SCALER_YUV420_sp;

	//destination setting
	phy_addr = pmap_scaler_dst.base;
	dst.addr_y = GET_ADDR_YUV42X_spY(phy_addr);
	dst.addr_u = GET_ADDR_YUV42X_spU(dst.addr_y, dst_width, dst_height);
	dst.addr_v = GET_ADDR_YUV420_spV(dst.addr_u, dst_width, dst_height);
	dst.x = 0;
	dst.y = 0;
	dst.w = dst_width;
	dst.h = dst_height;
	dst.w_x = 0;
	dst.w_y = 0;
	dst.w_w = dst_width;
	dst.w_h = dst_height;
	dst.format = SCALER_YUV420_sp;

	//executing scaler
	if(tcc_scaler_execute(scaler_fd, &src, &dst) < 0)
	{
		printf("scaler error!!\n");
		return -1;
	}

	//write to file
	fwrite(scaler_dst_ptr, 1, (dst_width * dst_height * 3) / 2, output_fp);
	fclose(input_fp);
	fclose(output_fp);

	munmap(scaler_ptr, pmap_scaler.size);
	munmap(scaler_dst_ptr, pmap_scaler_dst.size);

	//scaler driver close
	tcc_scaler_close(scaler_fd);

	printf("test done!\n");
	return 0;
}

#endif

