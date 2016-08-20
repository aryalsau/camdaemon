#ifndef _COMPASS
#define _COMPASS

int init_compass(void);
int uninit_compass(void);
int acquire_compass_fielddata(struct Data* data);

#endif
