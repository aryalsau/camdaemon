#include <syslog.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "../common.h"
#include "../camera.h"
#include "../compass/compass.h"


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
	syslog(LOG_INFO, "capture\n");
	if (VERBOSE) printf("capture\n");
	return 0;
}


int capture_write(struct Command* command, char* *response){
	struct Data data;
	int file_path_length;

	syslog(LOG_INFO, "capture_write\n");
	if (VERBOSE) printf("capture_write\n");

	capture(command, &data);
	write_to_disk(&data, *response);
	free_frame(&(data.imagedata), &(data.xdim), &(data.ydim));

	file_path_length = strlen(data.file_path);
	*response = (char*)malloc(file_path_length+2);
	sprintf(*response, "%s\n", data.file_path);

	free_data(&data);

	return 0;
}


// camera specific funcitons


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


int acquire_camera_imagedata(struct Data* data) {
	syslog(LOG_ERR, "retrieving image data\n");
	if (VERBOSE) printf("retrieving image data\n");
	data->xdim = 1024/data->xbin;
	data->ydim = 1024/data->ybin;
	allocate_frame(&(data->imagedata), &(data->xdim), &(data->ydim));
	for (short ii = 0; ii < data->xdim; ii++)
		for (short jj = 0; jj < data->ydim; jj++)
			(data->imagedata)[ii][jj] = jj;
	return 0;
}


int acquire_camera_temp(struct Data* data) {
	syslog(LOG_ERR, "retrieving camera temperature\n");
	if (VERBOSE) printf("retrieving camera temperature\n");
	data->temp_c = 20.0;
	return 0;
}
