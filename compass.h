#ifndef _COMPASS
#define _COMPASS

int init_compass(void);
int uninit_compass(void);
int acquire_compass_fielddata(float* grav_field, float* mag_field);

#endif
