#include <syslog.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "../common.h"
#include "../camera.h"


int init_camera(void){
	syslog(LOG_INFO, "initialising camera\n");
	if (VERBOSE) printf("initialising camera\n");
	return 1;
}


int uninit_camera(void){
	syslog(LOG_INFO, "uninitialising camera\n");
	if (VERBOSE) printf("uninitialising camera\n");
	return 0;
}


int acquire_camera_imagedata(struct Data* data) {
	syslog(LOG_INFO, "retrieving image data\n");
	if (VERBOSE) printf("retrieving image data\n");
	data->xdim = 1024/data->xbin;
	data->ydim = 1024/data->ybin;
	allocate_frame(&(data->imagedata), data->xdim, data->ydim);
	for (short ii = 0; ii < data->xdim; ii++)
		for (short jj = 0; jj < data->ydim; jj++)
			(data->imagedata)[ii+(data->xdim)*jj] = ii;
	return 0;
}


int acquire_camera_temp(struct Data* data) {
	syslog(LOG_INFO, "retrieving camera temperature\n");
	if (VERBOSE) printf("retrieving camera temperature\n");
	data->temp_c = 20.0;
	return 0;
}
