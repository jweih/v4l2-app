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
	sc_fd = open("/dev/scaler", O_RDWR | O_NDELAY);

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
	close(sc_fd);

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
	int mM2m_fd;
	SCALER_TYPE ScaleInfo;
	struct pollfd poll_event[1];

	DEBUG (DEB_LEV_SIMPLE_SEQ,"tcc_scaler_yuv420_full(%d,%d,%d,%d,%d,%d,%d,%d)\n",s_hsize,s_vsize,so_hsize,so_vsize,m_hsize,m_vsize,do_hsize,do_vsize);
	DEBUG (DEB_LEV_SIMPLE_SEQ,"(0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",s_y,s_u,s_v,d_y,d_u,d_v);

	mM2m_fd = open( TCC_SCALER_DEV0_NAME, O_RDWR | O_NDELAY);
	if (mM2m_fd <= 0)
	{
		DEBUG (DEB_LEV_TCC_ERR,"can't open'%s'",  TCC_SCALER_DEV0_NAME);
		return -1;
	}
	else
	{
	//	DEBUG (DEB_LEV_TCC_ERR,"m2m sclaer open ok\n");
	}

#if (0)
	filt2d_test(mM2m_fd);
#endif

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
			ScaleInfo.src_winLeft = 0;

		if( (do_vsize - m_vsize) != 0)
			ScaleInfo.src_winTop = (so_vsize - s_vsize)>>1;
		else
			ScaleInfo.src_winTop = 0;

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
		return -1;
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
			return -1;
		}
		else if (ret == 0) 
		{
			DEBUG (DEB_LEV_TCC_ERR,"m2m poll timeout\n");	
			close(mM2m_fd);
			return -1;
		}
		else if (ret > 0) 
		{
			if (poll_event[0].revents & POLLERR) 
			{
				DEBUG (DEB_LEV_TCC_ERR,"m2m poll POLLERR\n");
				close(mM2m_fd);
				return -1;
			}
		}

		close(mM2m_fd);
	}

	return ret;
}
