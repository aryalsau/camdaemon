#include <syslog.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <errno.h>
#include <fitsio.h>
#include "common.h"
#include "camera.h"
#include "compass/compass.h"


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


int update_config(char* config_filepath, struct Data* data){

	FILE *config_file;
	size_t line_size = 100;
	char* line = malloc(line_size);
	char* parameter_str;
	char* value_str;

	config_file = fopen(config_filepath, "r");
	if(config_file == NULL) {
		syslog(LOG_ERR, "Error opening file config.cfg\n");
		if (VERBOSE) printf("Error opening file config.cfg\n");
	} else {

		while (fgets(line, line_size, config_file) != NULL)  {
			parameter_str = strtok(line, "=");
			value_str = strtok(NULL, "\n");
			if ((strcmp(parameter_str, "SITE") == 0) || (strcmp(parameter_str, "site") == 0)){
				data->site = (char*)malloc(strlen(value_str)+1);
				strcpy(data->site, value_str);
			} else if ((strcmp(parameter_str, "CAMERA") == 0) || (strcmp(parameter_str, "camera") == 0)){
				data->camera = (char *)malloc(strlen(value_str)+1);
				strcpy(data->camera, value_str);
			} else if ((strcmp(parameter_str, "PATH") == 0) || (strcmp(parameter_str, "path") == 0)){
				data->location = (char *)malloc(strlen(value_str)+1);
				strcpy(data->location, value_str);
			}
		}
		free(line);
		fclose(config_file);

	}
	return 0;
}


int update_exp_time(unsigned long* exp_time_us, struct Data* data){
	data->exp_time_us = *exp_time_us;
	return 0;
}


int update_xbin(unsigned char* xbin, struct Data* data){
	data->xbin = *xbin;
	return 0;
}


int update_ybin(unsigned char* ybin, struct Data* data){
	data->ybin = *ybin;
	return 0;
}


int update_file_name(struct Data* data){
	int file_path_length, folder_path_length;
	time_t raw_time;
	time(&raw_time);
	data->time_info = gmtime(&raw_time);

	data->file_name = (char*)malloc(19);
	strftime(data->file_name, 19, "img%j_%H%M%S.fits", data->time_info);

	data->folder_name = (char*)malloc(8);
	strftime(data->folder_name, 8, "%b%d%y", data->time_info);

	folder_path_length = strlen(data->location)+1+8+1;
	data->folder_path = (char*)malloc(folder_path_length+1);
	sprintf(data->folder_path, "%s/%s/", data->location, data->folder_name);

	file_path_length = strlen(data->location)+1+8+1+19;
	data->file_path = (char*)malloc(file_path_length+1);
	sprintf(data->file_path, "%s/%s/%s", data->location, data->folder_name, data->file_name);

	return 0;
}


int allocate_frame(unsigned short** *frame, unsigned short* xdim, unsigned short* ydim) {
	*frame = (unsigned short**)malloc(*xdim*sizeof(unsigned short*));
	for(short i=0; i<*xdim; i++)
		(*frame)[i] = (unsigned short*)malloc(*ydim*sizeof(unsigned short));
	return 0;
}


int free_frame(unsigned short** *frame, unsigned short* xdim, unsigned short* ydim) {
	for(short i=0; i<*xdim; i++)
		free((*frame)[i]);
	free(*frame);
	return 0;
}


int write_to_disk(struct Data* data, char* response){

	int status;
	fitsfile *fptr;
	long nelements = data->xdim * data->ydim;
	long naxes[2] = {data->xdim, data->ydim};
	long naxis = 2;
	long fpixel = 1;

	mkdir(data->folder_path, 0700);

	status = 0;
	fits_create_file(&fptr, data->file_path, &status);
	fits_create_img(fptr, SHORT_IMG, naxis, naxes, &status);

	fits_update_key(fptr, TLONG, "EXPOSURE", &(data->exp_time_us), "Total Exposure Time", &status);
	fits_write_key_unit(fptr, "EXPOSURE", "us", &status);

	fits_update_key(fptr, TFLOAT, "TEMPERATURE", &(data->temp_c), "Camera Temperature", &status);
	fits_write_key_unit(fptr, "EXPOSURE", "us", &status);

	fits_update_key(fptr, TSHORT, "XBIN", &(data->xbin), "x binning", &status);

	fits_update_key(fptr, TSHORT, "YBIN", &(data->ybin), "y binning", &status);

	fits_update_key(fptr, TSTRING, "SITE", data->site, "Instrument Location", &status);

	fits_update_key(fptr, TSTRING, "CAMERA", data->camera, "Instrument Camera", &status);

	char *fitsdate = (char *)malloc(24);
	strftime(fitsdate, 24,"%Y-%m-%dT%H:%M:%S UT", data->time_info);
	fits_update_key(fptr, TSTRING, "DATE", fitsdate, "file creation date (YYYY-MM-DDThh:mm:ss UT)", &status);

	unsigned short array[naxes[0]][naxes[1]];
	for (unsigned short ii = 0; ii < naxes[0]; ii++)
		for (unsigned short jj = 0; jj < naxes[1]; jj++)
			array[ii][jj] = (data->imagedata)[ii][jj];

	fits_write_img(fptr, TSHORT, fpixel, nelements, array[0], &status);

	char *columns[] = {"ACCELERATION", "FIELD"};
	char *formats[] = {"3E","3E"};
	char *units[] = {"m/s/s","G"};
	int n_rows    = 3;
	int n_fields = 2;

	fits_create_tbl(fptr, BINARY_TBL, 0, n_fields, columns, formats, units, "COMPASS_DATA", &status);
	fits_write_col(fptr, TDOUBLE, 1, 1, 1, n_rows, data->grav_field, &status);
	fits_write_col(fptr, TDOUBLE, 2, 1, 1, n_rows, data->mag_field, &status);

	fits_close_file(fptr, &status);

	fits_report_error(stderr, status);

	return 0;
}


int free_data(struct Data* data){
	free(data->site);
	free(data->camera);
	free(data->location);
	free(data->file_name);
	free(data->folder_name);
	free(data->folder_path);
	free(data->file_path);
	return 0;
}
