#include <phidget21.h>
#include <syslog.h>
#include <stdbool.h>
#include <stdio.h>
#include "../common.h"
#include "compass.h"

int acquire_compass_fielddata(struct Data* data){
	syslog(LOG_ERR, "compass\n");
	if (VERBOSE) printf("compass\n");
	return 0;
}
