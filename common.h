#ifndef _COMMON
	#define _COMMON

	#include <stdbool.h>

	extern bool VERBOSE;

	enum {CAPTURE, STOP, INVALID} cmd;

	extern struct cmd_struct {
		cmd flag;
		unsigned long time_us;
		unsigned char binning[2];
	};

#endif
