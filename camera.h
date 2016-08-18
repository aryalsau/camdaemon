#ifndef _CAMERA
#define _CAMERA


int init_camera(void);
int uninit_camera(void);
int acquire_camera_imagedata(struct Data* data);
int acquire_camera_temp(struct Data* data);


#endif
