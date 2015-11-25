#include "common.h"


extern struct header build_header();
extern void copy_header(struct header header_struct, unsigned char *ptr_fileheader);
extern struct file_path file_path_string();
extern struct config read_config();
extern short ** dyanmically_allocate(short xdim, short ydim, int type_size);

extern struct config read_config(){

	struct config config_obj;

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
			config_obj.site = (char *)malloc(strlen(val_str));
			strcpy(config_obj.site, val_str);
		} else if (strcmp(parameter_str, "CAMERA") == 0){
			config_obj.camera = (char *)malloc(strlen(val_str));
			strcpy(config_obj.camera, val_str);
		} else if (strcmp(parameter_str, "PATH") == 0){
			config_obj.path = (char *)malloc(strlen(val_str));
			strcpy(config_obj.path, val_str);
		}
	}
	fclose(cfg_file);
	return config_obj;
}

extern struct file_path file_path_string() {

	struct file_path file_path_obj;

	time_t raw_time;
	struct tm *time_info;
	time(&raw_time);
	time_info = localtime(&raw_time);

	file_path_obj.filename = (char *)malloc(20);
	strftime(file_path_obj.filename, 20,"img%H%M%S_%j.fits", time_info);

	file_path_obj.folder_name = (char *)malloc(8);
	strftime(file_path_obj.folder_name,8,"%b%d%y", time_info);

	return file_path_obj;
}

extern short ** dyanmically_allocate(short xdim, short ydim, int type_size){
	short **image;
	image = (short **)malloc(xdim*type_size);
	short i;
	for (i = 0; i < xdim; i++) {
		image[i] = malloc(ydim*type_size);
	}
	return image;
}
