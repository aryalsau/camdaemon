#include "common.h"


extern struct header build_header();
extern void copy_header(struct header header_struct, unsigned char *ptr_fileheader);
extern struct file_path file_path_string();
extern struct config read_config();
extern short ** dyanmically_allocate(short xdim, short ydim, int type_size);
extern char * write_file(struct data data_object, char *location);

extern struct config read_config(){

	struct config config_object;

	FILE *cfg_file;
	cfg_file = fopen("config.cfg" , "r");
	if(cfg_file == NULL) {
		perror("Error opening file config.cfg");
		syslog( LOG_INFO, "Error opening file config.cfg\n");
	}

	int line_size = 256;
	char line[line_size];
	char *parameter_str;
	char *val_str;
	while( fgets (line, line_size, cfg_file)!=NULL ) {
		parameter_str = strtok(line, "=");
		val_str = strtok(NULL, "\n");
		if (strcmp(parameter_str, "SITE") == 0){
			config_object.site = (char *)malloc(strlen(val_str));
			strcpy(config_object.site, val_str);
		} else if (strcmp(parameter_str, "CAMERA") == 0){
			config_object.camera = (char *)malloc(strlen(val_str));
			strcpy(config_object.camera, val_str);
		} else if (strcmp(parameter_str, "PATH") == 0){
			config_object.path = (char *)malloc(strlen(val_str));
			strcpy(config_object.path, val_str);
		}
	}
	fclose(cfg_file);
	return config_object;
}

extern struct file_path file_path_string() {

	struct file_path file_path_object;

	time_t raw_time;
	struct tm *time_info;
	time(&raw_time);
	time_info = localtime(&raw_time);

	file_path_object.filename = (char *)malloc(18);
	strftime(file_path_object.filename, 18,"img%H%M%S_%j.fits", time_info);

	file_path_object.folder_name = (char *)malloc(8);
	strftime(file_path_object.folder_name,8,"%b%d%y", time_info);

	return file_path_object;
}

extern short ** dyanmically_allocate(short xdim, short ydim, int type_size){
	short **image;
	image = (short **)malloc(xdim*type_size);
	short i;
	for (i = 0; i < xdim; i++)
		image[i] = malloc(ydim*type_size);
	return image;
}

extern char * write_file(struct data data_object, char *location){
	int status;
	long nelements = data_object.xdim*data_object.ydim;
	long naxis = 2;
	long naxes[2] = {data_object.xdim,data_object.ydim};   /* image is 300 pixels wide by 200 rows */
	long fpixel = 1;

	struct stat st = {0};
	if (stat(location, &st) == -1) mkdir(location, 0700);

	char * full_path = (char *)malloc( strlen(location)+strlen(data_object.filename)+1);
	sprintf(full_path, "%s/%s", location, data_object.filename);

	fitsfile *fptr;       /* pointer to the FITS file; defined in fitsio.h */

	status = 0;
	fits_create_file(&fptr, full_path, &status);   /* create new file */
	fits_create_img(fptr, SHORT_IMG, naxis, naxes, &status);

	fits_update_key(fptr, TLONG, "EXPOSURE", &data_object.exp_time_ms, "Total Exposure Time", &status);
	fits_write_key_unit(fptr, "EXPOSURE", "ms", &status);

	fits_update_key(fptr, TFLOAT, "TEMPERATURE", &data_object.temperature, "Camera Temperature", &status);
	fits_write_key_unit(fptr, "TEMPERATURE", "C", &status);

	fits_update_key(fptr, TSTRING, "SITE", data_object.site, "Instrument Location", &status);

	fits_update_key(fptr, TSTRING, "CAMERA", data_object.camera, "Instrument Camera", &status);

	fits_write_date(fptr, &status);

	short ii, jj;
	short copy[data_object.xdim][data_object.ydim];
	for (jj = 0; jj < data_object.xdim; jj++)
		for (ii = 0; ii < data_object.ydim; ii++)
			copy[ii][jj] = data_object.image[ii][jj];

	/* Write the array of integers to the image */
	fits_write_img(fptr, TSHORT, fpixel, nelements, copy[0], &status);

	fits_close_file(fptr, &status);            /* close the file */

	fits_report_error(stderr, status);  /* print out any error messages */
	return full_path;
}
