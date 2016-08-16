#ifndef _COMMON
	#define _COMMON

	#include <stdbool.h>

	extern bool VERBOSE;

	typedef enum {CAPTURE, STOP, INVALID} Flag;

	struct Command {
		Flag flag;
		unsigned long time_us;
		unsigned char binning[2];
	};

#endif
