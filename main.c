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

/* for auto_start */
static int auto_start;

/**************************************************************************/
/*                         Util functions                                 */
/**************************************************************************/
char *tcc_malloc_string(char *sz)
{
    char *ret = NULL;
    unsigned int len = 0;

    if (sz) {
        len = strlen(sz);
        ret = (char *)malloc((len * sizeof(char)) + sizeof(char));
        if (ret) {
            strcpy(ret, sz);
        }
    }
    return ret;
}

void terminal_changemode(int nonblock)
{
    struct termios ttystate;

     // get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);

    if (nonblock) {
        // turn off canonical mode
        ttystate.c_lflag &= ~ICANON;
        // minimum of number input read
        ttystate.c_cc[VMIN] = 1;
    } else {
        // turn on canonical mode
        ttystate.c_lflag |= ICANON;
    }

    // set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

char wait_press_anykey(void)
{
    fd_set std_in;
    int ret;
    char key;

    terminal_changemode(1);

    FD_ZERO(&std_in);
    FD_SET(STDIN_FILENO, &std_in);


    ret = select(STDIN_FILENO + 1, &std_in, NULL, NULL, NULL);
    if (ret == -1)
    {
        terminal_changemode(0);
        return -1;
    }

    key = fgetc(stdin);

    printf("\n");
    terminal_changemode(0);
    return key;
}

unsigned int select_preview_format()
{
	unsigned int fmt = 0;

	do {
		char key;

		printf("Select Preview Image Format (press '0'~'5')\n");
		printf("  0: YCbCr 4:2:0 Separated (DEFAULT)\n");
		printf("  1: YCbCr 4:2:2 Separated\n");
		printf("  2: YCbCr 4:2:0 Interleaved Type0\n");
		printf("  3: YCbCr 4:2:0 Interleaved Type1\n");
		printf("  4: YCbCr 4:2:2 Interleaved Type0\n");
		printf("  5: YCbCr 4:2:2 Interleaved Type1\n");
		printf("  6: YCbCr 4:2:2 UYVY Sequential\n");
		printf("  7: YCbCr 4:2:2 VYUY Sequential\n");
		printf("  8: YCbCr 4:2:2 YUYV Sequential\n");
		printf("  9: YCbCr 4:2:2 YVYU Sequential\n");
		printf("  a: RGB   8:8:8 3bytes\n");

		usleep(100000);
		key = wait_press_anykey();

		switch (key) {
		case '0':
			fmt = TCC_LCDC_IMG_FMT_YUV420SP;
			break;
		case '1':
			fmt = TCC_LCDC_IMG_FMT_YUV422SP;
			break;
		case '2':
			fmt = TCC_LCDC_IMG_FMT_YUV420ITL0;
			break;
		case '3':
			fmt = TCC_LCDC_IMG_FMT_YUV420ITL1;
			break;
		case '4':
			fmt = TCC_LCDC_IMG_FMT_YUV422ITL0;
			break;
		case '5':
			fmt = TCC_LCDC_IMG_FMT_YUV422ITL1;
			break;
		case '6':
			fmt = TCC_LCDC_IMG_FMT_UYVY;
			break;
		case '7':
			fmt = TCC_LCDC_IMG_FMT_VYUY;
			break;
		case '8':
			fmt = TCC_LCDC_IMG_FMT_YUYV;
			break;
		case '9':
			fmt = TCC_LCDC_IMG_FMT_YVYU;
			break;
		case 'a':
			fmt = TCC_LCDC_IMG_FMT_RGB888_3;	// VIOC_IMG_FMT_RGB888
			break;
		default:
			break;
		}
	} while (fmt == 0);

	return fmt;
}

void i2c_send_8(CameraDevice *dev, unsigned char reg, unsigned char val)
{
	int ret;
	unsigned char data[2];
	struct i2c_rdwr_ioctl_data i2c_data;
	struct i2c_msg msg[1];

	data[0] = reg;
	data[1] = val;

	i2c_data.msgs = msg;
	i2c_data.nmsgs = 1;
	i2c_data.msgs[0].addr = dev->i2c_slave_addr;
	i2c_data.msgs[0].flags = 0;
	i2c_data.msgs[0].len = sizeof(data);
	i2c_data.msgs[0].buf = data;

	ret = ioctl(dev->i2c_fd, I2C_SLAVE_FORCE, dev->i2c_slave_addr);
	if (ret < 0) {
		printf("i2c(%d): I2C_SLAVE_FORCE(0x%x) failed\n", dev->i2c_fd, dev->i2c_slave_addr);
	}

	ret = ioctl(dev->i2c_fd, I2C_RDWR, &i2c_data);
	if (ret < 0) {
		printf("i2c: I2C_RDWR write failed\n");
	}

	printf("[i2c send] reg(0x%02x) val(0x%02x)\n", data[0], data[1]);
}

void i2c_recv_8(CameraDevice *dev, unsigned char reg)
{
	int ret;
	unsigned char data[2];
	struct i2c_rdwr_ioctl_data i2c_data;
	struct i2c_msg msg[2];

	data[0] = reg;

	i2c_data.msgs = msg;
	i2c_data.nmsgs = 2;	// 1st is write offset, 2nd is read value
	/* write msg */
	i2c_data.msgs[0].addr = dev->i2c_slave_addr;
	i2c_data.msgs[0].flags = 0;
	i2c_data.msgs[0].len = 1;
	i2c_data.msgs[0].buf = &data[0];
	/* read msg */
	i2c_data.msgs[1].addr = dev->i2c_slave_addr;
	i2c_data.msgs[1].flags = I2C_M_RD;
	i2c_data.msgs[1].len = 1;
	i2c_data.msgs[1].buf = &data[1];

	ret = ioctl(dev->i2c_fd, I2C_SLAVE_FORCE, dev->i2c_slave_addr);
	if (ret < 0) {
		printf("i2c(%d): I2C_SLAVE_FORCE(0x%x) failed\n", dev->i2c_fd, dev->i2c_slave_addr);
	}

	ret = ioctl(dev->i2c_fd, I2C_RDWR, &i2c_data);
	if (ret < 0) {
		printf("i2c: I2C_RDWR read failed\n");
	}

	printf("[i2c recv] reg(0x%02x) val(0x%02x)\n", data[0], data[1]);
}

void i2c_send_16(CameraDevice *dev, unsigned short reg, unsigned char val)
{
	int ret;
	unsigned char data[3];
	struct i2c_rdwr_ioctl_data i2c_data;
	struct i2c_msg msg[1];

	data[0] = reg >> 8;
	data[1] = reg & 0xff;
	data[2] = val;

	i2c_data.msgs = msg;
	i2c_data.nmsgs = 1;
	i2c_data.msgs[0].addr = dev->i2c_slave_addr;
	i2c_data.msgs[0].flags = 0;
	i2c_data.msgs[0].len = sizeof(data);
	i2c_data.msgs[0].buf = data;

	ret = ioctl(dev->i2c_fd, I2C_SLAVE_FORCE, dev->i2c_slave_addr);
	if (ret < 0) {
		printf("i2c(%d): I2C_SLAVE_FORCE(0x%x) failed\n", dev->i2c_fd, dev->i2c_slave_addr);
	}

	ret = ioctl(dev->i2c_fd, I2C_RDWR, &i2c_data);
	if (ret < 0) {
		printf("i2c: I2C_RDWR write failed\n");
	}

	printf("[i2c send] reg(0x%02x%02x) val(0x%02x)\n", data[0], data[1], data[2]);
}

void i2c_recv_16(CameraDevice *dev, unsigned short reg)
{
	int ret;
	unsigned char data[3];
	struct i2c_rdwr_ioctl_data i2c_data;
	struct i2c_msg msg[2];

	data[0] = reg >> 8;
	data[1] = reg & 0xff;

	i2c_data.msgs = msg;
	i2c_data.nmsgs = 2;	// 1st is write offset, 2nd is read value
	/* write msg */
	i2c_data.msgs[0].addr = dev->i2c_slave_addr;
	i2c_data.msgs[0].flags = 0;
	i2c_data.msgs[0].len = 2;
	i2c_data.msgs[0].buf = &data[0];
	/* read msg */
	i2c_data.msgs[1].addr = dev->i2c_slave_addr;
	i2c_data.msgs[1].flags = I2C_M_RD;
	i2c_data.msgs[1].len = 1;
	i2c_data.msgs[1].buf = &data[2];

	ret = ioctl(dev->i2c_fd, I2C_SLAVE_FORCE, dev->i2c_slave_addr);
	if (ret < 0) {
		printf("i2c(%d): I2C_SLAVE_FORCE(0x%x) failed\n", dev->i2c_fd, dev->i2c_slave_addr);
	}

	ret = ioctl(dev->i2c_fd, I2C_RDWR, &i2c_data);
	if (ret < 0) {
		printf("i2c: I2C_RDWR read failed\n");
	}

	printf("[i2c recv] reg(0x%02x%02x) val(0x%02x)\n", data[0], data[1], data[2]);
}


/*===========================================================================
FUNCTION
===========================================================================*/
void* handle_camera(void *param)
{
    CameraDevice *dev = (CameraDevice *)param;

	while (1) {
		if(dev->cam_mode == MODE_PREVIEW)
		    camif_get_frame(dev);
		else
			rsc_sched_delay(100);
	}

	return NULL;
}

/*===========================================================================
FUNCTION
===========================================================================*/
void* handle_stdin(void* param)
{
	CameraDevice *dev = (CameraDevice *)param;
	char cmdline[1024];
	int r;

	if (auto_start)
		goto auto_start;

	while (1)
	{
		usleep(1000*100);

		if (auto_start)
			continue;

		// 1. Get Command!!
		memset(cmdline,0x00,1024);		
		if((r = read(STDIN_FILENO, cmdline, 1024)) == 0) 
			printf(" Wrong input!! \n");

		// 2. Process Function!!
		if (!strncmp("start", cmdline, 5))
		{
auto_start:
			/* dispaly on */
			dev->display = 1;

			rsc_init_lcd(dev);

			camif_set_resolution(dev, dev->preview_width, dev->preview_height);
			camif_start_stream(dev);
			rsc_overlay_ctrl(dev, 1);
		}
		else if (!strncmp("quit", cmdline, 4)) 
		{
			exit(1);
		}
		else if (!strncmp("stop", cmdline, 4)) 
		{
			camif_stop_stream(dev);
			rsc_overlay_ctrl(dev, 0);
		}
		/*----------------------------------------------------------------
		 * V4L2_CID_XXX Control
		 */
		else if (!strncmp("exposure", cmdline, 8))
		{
			char *args = cmdline + 9;
			int val = atoi(args);
			printf("[V4L2_CID_EXPOSURE] set value %d\n", val);
			camif_set_queryctrl(dev, V4L2_CID_EXPOSURE, val);
		}
		else if (!strncmp("awb", cmdline, 3))
		{
			char *args = cmdline + 4;
			int val = atoi(args);
			printf("[V4L2_CID_AUTO_WHITE_BALANCE] set value %d\n", val);
			camif_set_queryctrl(dev, V4L2_CID_AUTO_WHITE_BALANCE, val);
		}
		else if (!strncmp("contrast", cmdline, 8))
		{
			char *args = cmdline + 9;
			int val = atoi(args);
			printf("[V4L2_CID_CONTRAST] set value %d\n", val);
			camif_set_queryctrl(dev, V4L2_CID_CONTRAST, val);
		}
		else if (!strncmp("saturation", cmdline, 10))
		{
			char *args = cmdline + 11;
			int val = atoi(args);
			printf("[V4L2_CID_SATURATION] set value %d\n", val);
			camif_set_queryctrl(dev, V4L2_CID_SATURATION, val);
		}
		else if (!strncmp("rotate", cmdline, 6))
		{
			char *args = cmdline + 7;
			int val = atoi(args);
			printf("[V4L2_CID_ROTATE] set value %d\n", val);
			camif_set_queryctrl(dev, V4L2_CID_ROTATE, val);
		}
		else if (!strncmp("framerate", cmdline, 9))
		{
			char *args = cmdline + 10;
			int val = atoi(args);
			printf("[V4L2_CID_FRAMERATE] set value %d\n", val);
			camif_set_queryctrl(dev, V4L2_CID_FRAMERATE, val);
		}
		else if (!strncmp("scene", cmdline, 5))
		{
			char *args = cmdline + 6;
			int val = atoi(args);
			printf("[V4L2_CID_SCENE] set value %d\n", val);
			camif_set_queryctrl(dev, V4L2_CID_SCENE, val);
		}
		else if (!strncmp("frameskip", cmdline, 9))
		{
			char *args = cmdline + 10;
			int val = atoi(args);
			printf("[V4L2_CID_FRAMESKIP] set value %d\n", val);
			camif_set_queryctrl(dev, V4L2_CID_FRAMESKIP, val);
		}
		/* TEST */
		else if (!strncmp("test1", cmdline, 5))
		{
			char *args = cmdline + 6;
			int val = atoi(args);
			printf("test1: [V4L2_CID_ZOOM_ABSOLUTE] set value %d\n", val);
			camif_set_queryctrl(dev, V4L2_CID_ZOOM_ABSOLUTE, val);
		}
		else if (!strncmp("test2", cmdline, 5))
		{
			char *args = cmdline + 6;
			int val = atoi(args);
			unsigned int V4L2_CID_INVALID = 0xffffffff;
			printf("test2: [V4L2_CID_INVALID] test invalid ioctl code 0x%08X\n", V4L2_CID_INVALID);
			camif_set_queryctrl(dev, V4L2_CID_INVALID, val);
		}
		/*--------end of V4L2_CID_XXX Control -------------------------*/
		else if (!strncmp("overlay", cmdline, 7))
		{
			char *args = cmdline + 8;

			if (*(args) < '0' || *(args) > '1') 
			{
				printf("usage : overlay [overlay_value]\n");
				printf("ex) overlay 1\n");
				continue;
			}   
			camif_set_overlay(dev, *(args)-'0');
		}
		else if (!strncmp("capture", cmdline, 7)) 
		{	

		}
		/*----------------------------------------------------------------
		 * VIQE Histogram Control
		 */
		else if (!strncmp("histogram", cmdline, 9)) 
		{
			printf("[VIQE Histogram]\n");
			dev->histogram_excute = 1;
		}
		/*----------------------------------------------------------------
		 * I2C Control
		 */
		else if (!strncmp("w8", cmdline, 2)) 
		{
			/* 12 34 56
			 * w8 44 55
			 */
			char *args;
			unsigned char reg, val;

			args = cmdline + 3;
			reg = strtol(args, NULL, 16);	// offset of register
			args = cmdline + 5;
			val = strtol(args, NULL, 16);	// value

			i2c_send_8(dev, reg, val);
		}
		else if (!strncmp("r8", cmdline, 2)) 
		{
			/* 12 34
			 * r8 44 
			 */
			char *args;
			unsigned char reg;

			args = cmdline + 3;
			reg = strtol(args, NULL, 16);

			i2c_recv_8(dev, reg);
		}
		else if (!strncmp("w16", cmdline, 3)) 
		{
			/* 123 4567 89
			 * w16 4444 55 
			 */
			char *args;
			unsigned short reg;
			unsigned char val;

			args = cmdline + 4;
			reg = strtol(args, NULL, 16);	// offset of register
			args = cmdline + 8;
			val = strtol(args, NULL, 16);	// value

			i2c_send_16(dev, reg, val);
		}
		else if (!strncmp("r16", cmdline, 3)) 
		{
			/* 123 4567
			 * r16 4444 
			 */
			char *args;
			unsigned short reg;

			args = cmdline + 4;
			reg = strtol(args, NULL, 16);

			i2c_recv_16(dev, reg);
		}
		else
		{
			printf("invalid input\n\n");
		}
	}

	return NULL;
}

void* handle_stdin_multi(void* param)
{
	CameraDevice *dev = (CameraDevice *)param;
	char cmdline[32];
	int r;
	int i, first = 0;
	unsigned int prev_w, prev_h;
	unsigned int sx, sy, w, h, fmt;

	if (auto_start)
		goto auto_start;

	while (1)
	{		
		usleep(1000*100);

		if (auto_start)
			continue;
	
		// 1. Get Command!!
		memset(cmdline, 0, 32);		
		if((r = read(STDIN_FILENO, cmdline, 32)) == 0) 
			printf(" Wrong input!! \n");

		// 2. Process Function!!
		if (!strncmp("start", cmdline, 5))
		{
auto_start:
			for (i = 0; i < DEVICE_NR; i++) {
				if (dev[i].use) {
					rsc_init_lcd(&dev[i]);
					if (!first++) {
						prev_w = dev[i].preview_width;
						prev_h = dev[i].preview_height;
						sx = dev[i].overlay_config.sx;
						sy = dev[i].overlay_config.sy;
						w = dev[i].overlay_config.width;
						h = dev[i].overlay_config.height;
						fmt = dev[i].overlay_config.format;
					} else {
						dev[i].preview_width = prev_w;
						dev[i].preview_height = prev_h;
						dev[i].overlay_config.sx = sx;
						dev[i].overlay_config.sy = sy;
						dev[i].overlay_config.width = w;
						dev[i].overlay_config.height = h;
						dev[i].overlay_config.format = fmt;
					}

					camif_set_resolution(&dev[i], dev[i].preview_width, dev[i].preview_height);
					camif_start_stream(&dev[i]);

					if (first == 1) {
						rsc_overlay_ctrl(&dev[i], 1);
						dev[i].display = 1;		// default dispaly on
					}
				}
			}
		}
		else if (!strncmp("0", cmdline, 1))
		{
			dev[0].display = 1;		// video0 dispaly on
			dev[1].display = 0;
			dev[2].display = 0;
			dev[3].display = 0;
			dev[4].display = 0;
		}
		else if (!strncmp("1", cmdline, 1))
		{
			dev[0].display = 0;
			dev[1].display = 1;
			dev[2].display = 0;
			dev[3].display = 0;
			dev[4].display = 0;
		}
		else if (!strncmp("2", cmdline, 1))
		{
			dev[0].display = 0;
			dev[1].display = 0;
			dev[2].display = 1;
			dev[3].display = 0;
			dev[4].display = 0;
		}
		else if (!strncmp("3", cmdline, 1))
		{
			dev[0].display = 0;
			dev[1].display = 0;
			dev[2].display = 0;
			dev[3].display = 1;
			dev[4].display = 0;
		}
		else if (!strncmp("4", cmdline, 1))
		{
			dev[0].display = 0;
			dev[1].display = 0;
			dev[2].display = 0;
			dev[3].display = 0;
			dev[4].display = 1;
		}
		else if (!strncmp("stop", cmdline, 4)) 
		{
			for (i = 0; i < DEVICE_NR; i++) {
				if (dev[i].use) {
					camif_stop_stream(&dev[i]);
					rsc_overlay_ctrl(&dev[i], 0);
				}
			}
		}
		else if (!strncmp("quit", cmdline, 4)) 
		{
			exit(1);
		}
		/*----------------------------------------------------------------
		 * I2C Control
		 */
		else if (!strncmp("w8", cmdline, 4)) 
		{
			/* 12 34 56
			 * w8 44 55
			 */
			char *args;
			unsigned char reg, val;

			args = cmdline + 3;
			reg = strtol(args, NULL, 16);	// offset of register
			args = cmdline + 5;
			val = strtol(args, NULL, 16);	// value

			i2c_send_8(dev, reg, val);
		}
		else if (!strncmp("r8", cmdline, 4)) 
		{
			/* 12 34
			 * r8 44 
			 */
			char *args;
			unsigned char reg;

			args = cmdline + 3;
			reg = strtol(args, NULL, 16);

			i2c_recv_8(dev, reg);
		}
		else if (!strncmp("w16", cmdline, 3)) 
		{
			/* 123 4567 89
			 * w16 4444 55 
			 */
			char *args;
			unsigned short reg;
			unsigned char val;

			args = cmdline + 4;
			reg = strtol(args, NULL, 16);	// offset of register
			args = cmdline + 8;
			val = strtol(args, NULL, 16);	// value

			i2c_send_16(dev, reg, val);
		}
		else if (!strncmp("r16", cmdline, 3)) 
		{
			/* 123 4567
			 * r16 4444 
			 */
			char *args;
			unsigned short reg;

			args = cmdline + 4;
			reg = strtol(args, NULL, 16);

			i2c_recv_16(dev, reg);
		}
		else
		{
			printf("invalid input\n\n");
		}
	}

	return NULL;
}

/*===========================================================================
FUNCTION
===========================================================================*/
void help_msg(void)
{
	printf("------------------------------------------------------------\n"
           "|  Usage: camapp\n"
           "| ===============\n"
           "| <Display sensor image to LCD>\n" 
           "|  -d [device number...]\n"
           "|       0: /dev/video0\n"
           "|       1: /dev/video1\n"
           "|       2: /dev/video2\n"
           "|       3: /dev/video3\n"
           "|       4: /dev/video4\n"
           "|    ex) display sensor /dev/video0\n"
           "|       $ camapp -d 0\n"
           "|    ex) display sensor /dev/video0, 1, 2, 3\n"
           "|       $ camapp -d 0 1 2 3\n"
           "|\n"
           "| ------ for debugging options -----\n"
           "| <Time-Stamp & Recoding file>\n"
           "|  * ONLY support single channel\n"
           "|  -t [option]\n"
           "|       t: display timestamp\n"
           "|       r: recording file (rec_data)\n"
           "|    ex) display timestamp\n"
           "|       $ camapp -d 0 -t t\n"
           "|    ex) recording rec_data file\n"
           "|       $ camapp -d 0 -t r\n"
           "|\n"
           "| <I2C Control for debuggind sensor module>\n"
           "|    -a [i2c slave 7bit addr (hex)]\n"
           "|    -p [i2c dev node (string)]\n"
           "|        ex) i2c slave addr is '0x5d', i2c dev node is '/dev/i2c-2'\n"
           "|            $ camapp -d 0 -a 5d -p /dev/i2c-2\n"
           "|      * After camapp stream starting, you can use i2c send/recv command\n"
           "|        ex 1-a) write 8bit register 0x8c, 8bit value 0x1f\n"
           "|            $ w8 8c 1f\n"
           "|           1-b) read 8bit register 0x8c\n"
           "|            $ r8 8c\n"
           "|        ex 2-a) write 16bit register 0x8c12, 8bit value 0x1f\n"
           "|            $ w16 8c12 1f\n"
           "|           2-b) read 16bit register 0x8c12\n"
           "|            $ r16 8c12\n"
           "|\n"
           "------------------------------------------------------------\n");
}

static int use_vout = 0;
int parse_args(int argc, char *argv[], CameraDevice *dev, int *single, int *single_nr, int *option)
{
	int ret = 0;
	int device = 0;
	char *mode;
	int width = 0, height = 0;
	int i2c_slave_addr = 0;
	char *i2c_dev_port = NULL;
	static struct option long_opt[] = {
		{"device", 1, 0, 'd'},
		{"out_width", 1, 0, 'w'},
		{"out_height", 1, 0, 'h'},
		{"option", 1, 0, 't'},
		{"i2c_addr", 1, 0, 'a'},
		{"i2c_port", 1, 0, 'p'},
		{0, 0, 0, 0}
	};

	while (1) {
		int c = 0;
		int option_idx = 0;

		c = getopt_long(argc, argv, "xdv:w:h:t:a:p:", long_opt, &option_idx);
		if (c == -1) { break; }

		switch (c) {
		case 0:
			break;
		case 'x':
			auto_start = 1;
			break;
		case 'd':
			device = 1;
			break;
		case 'v':
			use_vout = 1;
			break;
		case 'w':
			width = atoi(optarg);
			break;
		case 'h':
			height = atoi(optarg);
			break;
		case 't':
			mode = tcc_malloc_string(optarg);
			if (strcmp(mode, "t") == 0)
				*option = 1;
			else if (strcmp(mode, "r") == 0)
				*option = 2;
			else
				ret = -1;
			break;
		case 'a':
			i2c_slave_addr = strtol(optarg, NULL, 16);
			break;
		case 'p':
			i2c_dev_port = tcc_malloc_string(optarg);
			break;
		default:
			printf("invalid argument: optarg[%s]\n", optarg);
			ret = -1;
			break;
		}
	}

	if (device) {
		while (optind < argc) {
			int i = atoi(argv[optind]);
			if (i >= 0 && i < DEVICE_NR) {
				*single += 1;
				*single_nr = i;
				dev[i].use = 1;
				dev[i].output_width = width;
				dev[i].output_height = height;
				sprintf(dev[i].dev_name, "/dev/video%d", i);
				printf("%s\n", dev[i].dev_name);

				if (i2c_slave_addr) {
					dev[i].i2c_slave_addr = i2c_slave_addr;
					if (i2c_dev_port != NULL) {
						memcpy(dev[i].i2c_dev_port, i2c_dev_port, sizeof(dev[i].i2c_dev_port));
					} else {
						printf("error: need i2c device name\n");
						ret = -1;
					}
					printf("i2c: 0x%x, %s\n", dev[i].i2c_slave_addr, dev[i].i2c_dev_port);
				}
			} else {
				ret = -1;
			}
			optind++;
		}
	} else {
		ret = -1;
	}

exit:
	return ret;
}

int main(int argc, char *argv[])
{
	int i, /*arg = 0,*/ first = 0;
	int single = 0, single_nr = 0;
	int option = 0;
	CameraDevice dev[DEVICE_NR];
	unsigned int preview_fmt;
	int fb_fd, overlay_fd, composite_fd, viqe_fd, i2c_fd;

	if (argc < 2) {
		help_msg();
		return -1;
	}

	memset(dev, 0, sizeof(CameraDevice) * DEVICE_NR);

	if (parse_args(argc, argv, dev, &single, &single_nr, &option)) {
		help_msg();
		return -1;
	}

	/*
	 * choice preview image format
	 */
	if (auto_start) {
		preview_fmt = TCC_LCDC_IMG_FMT_YUV420SP;
		printf("Preview Image Format: YCbCr 4:2:0 Separated\n");
	} else {
		preview_fmt = select_preview_format();
	}

	if (use_vout && single > 1) {
		printf("error: V4L2 VOUT driver only supports single VIN driver.\n");
		return -1;
	}

    /*
     * Open Device (CAMERA, LCD, Etc..)
     */
	for (i = 0; i < DEVICE_NR; i++) {
		if (dev[i].use) {
			dev[i].camdev = i;
			dev[i].use_vout = use_vout;
			init_camera_data(&dev[i], preview_fmt, option);
			if ((dev[i].fd = open(dev[i].dev_name, O_RDWR)) < 0) {
				printf("error: driver open fail (%s)\n", dev[i].dev_name);
				return -1;
		    }

			if (!first++) {
				open_device(&dev[i]);
				fb_fd = dev[i].fb_fd0;
				overlay_fd = dev[i].overlay_fd;
				composite_fd = dev[i].composite_fd;
				viqe_fd = dev[i].viqe_fd;
				i2c_fd = dev[i].i2c_fd;
				camif_get_dev_info(&dev[i]);
			} else {
				dev[i].fb_fd0 = fb_fd;
				dev[i].overlay_fd = overlay_fd;
				dev[i].composite_fd = composite_fd;
				dev[i].viqe_fd = viqe_fd;
				dev[i].i2c_fd = i2c_fd;
			}

			/* Creat Thread (Get-Frame)
		     */
			pthread_create(&dev[i].frame_threads, NULL, handle_camera, (void *)&dev[i]);
			dev[i].cam_mode = MODE_START;
		}
    }

	printf("CAMERA Start Ready!!! \n");

	/*
	 * IPC-Main Loop
	 */
	if (single == 1) {
		handle_stdin(&dev[single_nr]);
	} else {
		handle_stdin_multi(dev);
	}

	/*
	 * Close Device/Thread..
	 */
	for (i = 0; i < 4; i++) {
		if (dev[i].use) {
			pthread_join(dev[i].frame_threads, 0);
		    close_device(&dev[i]);
		}
	}

    return 0;
}
