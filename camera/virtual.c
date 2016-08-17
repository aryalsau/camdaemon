#include <syslog.h>
#include <stdbool.h>
#include <stdio.h>
#include "../common.h"
#include "../camera.h"
#include "../compass/compass.h"


int init_camera(void){
	syslog(LOG_ERR, "initialising camera\n");
	if (VERBOSE) printf("initialising camera\n");
	return 1;
}

int uninit_camera(void){
	syslog(LOG_ERR, "uninitialising camera\n");
	if (VERBOSE) printf("uninitialising camera\n");
	return 0;
}

int capture(struct Command* command, struct Data* data){
	char config_filepath[] = "config.cfg";
	update_exptime(&(command->exp_time_us), data);
	update_xbin(&(command->xbin), data);
	update_ybin(&(command->ybin), data);
	update_config(config_filepath, data);
	update_filename(data);
	acquire_compass_fielddata(data);
	acquire_camera_imagedata(data);
	syslog(LOG_INFO, "capture command\n");
	if (VERBOSE) printf("capture command\n");
	return 0;
}

int capture_write(struct Command* command, char* response){
	struct Data data;
	syslog(LOG_INFO, "capture_write command\n");
	if (VERBOSE) printf("capture_write command\n");
	capture(command, &data);
	write_to_disk(&data, response);
	return 0;
}

int acquire_camera_imagedata(struct Data* data) {
	return 0;
}
