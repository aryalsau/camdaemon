#ifndef _CAMHELPER
	#define _CAMHELPER

	#include "../common.h"

	int init_camera();
	int uninit_camera();
	int capture(unsigned long exp_time_ms);
	int capture_write(struct Command* command, char* response);

#endif
