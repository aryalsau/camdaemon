#include <phidget21.h>
#include <syslog.h>
#include <stdbool.h>
#include <stdio.h>
#include "../common.h"
#include "compass.h"


int init_compass(void){
	syslog(LOG_ERR, "initialising compass\n");
	if (VERBOSE) printf("initialising compass\n");
	return 1;
}


int uninit_compass(void){
	syslog(LOG_ERR, "uninitialising compass\n");
	if (VERBOSE) printf("uninitialising compass\n");
	return 0;
}


int acquire_compass_fielddata(struct Data* data){
	syslog(LOG_ERR, "retrieving compass data\n");
	if (VERBOSE) printf("retrieving compass data\n");

	for (short i = 0; i<3; i++) {
		(data->grav_field)[i] = i;
		(data->mag_field)[i] = i;
	}

	return 0;
}
