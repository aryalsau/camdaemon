#ifndef _CAMERA
#define _CAMERA

int init_camera(void);
int uninit_camera(void);
int capture(struct Command* command, struct Data* data);
int capture_write(struct Command* command, char* *response);
int acquire_camera_imagedata(struct Data* data);
int acquire_camera_temp(struct Data* data);

#endif
