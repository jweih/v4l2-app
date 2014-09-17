/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

               Camera    I n t e r f a c e    M O D U L E

                        EDIT HISTORY FOR MODULE

when        who       what, where, why
--------    ---       -------------------------------------------------------
10/xx/08   ZzaU      Created file.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/


#ifndef __CAM_RSC_H__
#define __CAM_RSC_H__


/*===========================================================================
                           LCD InterFace
===========================================================================*/
extern void rsc_init_lcd(CameraDevice* self);
extern void rsc_set_lcd_ch0(CameraDevice* self, unsigned char en);
extern void rsc_set_lcd_addr(CameraDevice* self, unsigned int addr);
extern void rsc_draw_lcd(CameraDevice* self);

extern void rsc_video_push_vsync(CameraDevice* self, struct v4l2_buffer *pBuf);
extern void rsc_directly_draw_lcd(CameraDevice* self, struct v4l2_buffer *pBuf);

/*===========================================================================
                           G2D InterFace
===========================================================================*/
extern void rsc_process_image(CameraDevice *self, uint8_t *source);

  
/*===========================================================================
                           JPEG InterFace
===========================================================================*/
extern void  rsc_encode_jpeg(CameraDevice *self);


/*===========================================================================
                           FS InterFace
===========================================================================*/
extern void  rsc_save_file(CameraDevice *self, unsigned short* filename);


/*===========================================================================
                           Kernel Resource
===========================================================================*/
extern void rsc_sched_delay(int ms);
extern void rsc_sched_delay1(int ms);

/*===========================================================================
                           Etc..
===========================================================================*/
extern void rsc_buf_timestamp_logprint(struct v4l2_buffer *pBuf);


#endif /* __CAM_RSC_H__ */
