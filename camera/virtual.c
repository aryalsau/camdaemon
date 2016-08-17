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
	update_exp_time(&(command->exp_time_us), data);
	update_xbin(&(command->xbin), data);
	update_ybin(&(command->ybin), data);
	update_config(config_filepath, data);
	update_file_name(data);
	acquire_compass_fielddata(data);
	acquire_camera_temp(data);
	acquire_camera_imagedata(data);

	// printf("data->exp_time_us : %lu\n", data->exp_time_us);
	// printf("data->xbin : %d\n", data->xbin);
	// printf("data->ybin : %d\n", data->ybin);
	// printf("data->site : %s\n", data->site);
	// printf("data->camera : %s\n", data->camera);
	// printf("data->location : %s\n", data->location);
	// printf("data->file_name : %s\n", data->file_name);
	// printf("data->folder_name : %s\n", data->folder_name);
	// printf("data->full_path : %s\n", data->full_path);
	// printf("data->temp_c : %f\n", data->temp_c);

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

int acquire_camera_temp(struct Data* data) {
	data->temp_c = 20.0;
	return 0;
}
