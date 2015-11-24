#include "common.h"


extern struct header build_header();
extern void copy_header(struct header header_struct, unsigned char *ptr_fileheader);
extern struct file_path file_path_string();
extern struct config read_config();

int XDIM;
int YDIM;
int DAY;
int YEAR;
int XBIN;
int YBIN;
int NUMSUBARRAYS;
int LEFT;
int RIGHT;
int TOP1;
int BOTTOM1;
int NUMCOADDS;
long EXPTIMEMS;
long WAITTIMEMS;
float TEMPERATURE;
char * SITENAME;
char * FILENAME;
char * MONTH;
char * FILTER;

char * LOCATION;

extern struct header build_header(){
	struct header header_struct;
	header_struct.xdim = XDIM;
	header_struct.ydim = YDIM;
	header_struct.day = DAY;
	header_struct.year = YEAR;
	header_struct.xbin = XBIN;
	header_struct.ybin = YBIN;
	header_struct.num_subarrays = NUMSUBARRAYS;
	header_struct.left = LEFT;
	header_struct.right = RIGHT;
	header_struct.top1 = TOP1;
	header_struct.bottom1 = BOTTOM1;
	header_struct.num_coadds = NUMCOADDS;
	header_struct.exp_time = EXPTIMEMS/1000;
	header_struct.wait_time = WAITTIMEMS/1000;
	header_struct.temperature = TEMPERATURE;
	header_struct.site_name = SITENAME;
	header_struct.filename = FILENAME;
	header_struct.month = MONTH;
	header_struct.filter = FILTER;
	return header_struct;
}

extern void copy_header(struct header header_struct, unsigned char *ptr_fileheader) {
	memcpy(&ptr_fileheader[0], &header_struct.xdim, 2);
	memcpy(&ptr_fileheader[2], &header_struct.ydim, 2);
	memcpy(&ptr_fileheader[36], &header_struct.day, 2);
	memcpy(&ptr_fileheader[31], &header_struct.year, 2);
	memcpy(&ptr_fileheader[45], &header_struct.xbin, 2);
	memcpy(&ptr_fileheader[47], &header_struct.ybin, 2);
	memcpy(&ptr_fileheader[49], &header_struct.num_subarrays, 2);
	memcpy(&ptr_fileheader[51], &header_struct.left, 2);
	memcpy(&ptr_fileheader[53], &header_struct.right, 2);
	memcpy(&ptr_fileheader[55], &header_struct.bottom1, 2);
	memcpy(&ptr_fileheader[57], &header_struct.top1, 2);
	memcpy(&ptr_fileheader[75], &header_struct.num_coadds, 2);
	memcpy(&ptr_fileheader[81], &header_struct.exp_time, 4);
	memcpy(&ptr_fileheader[85], &header_struct.wait_time, 4);
	memcpy(&ptr_fileheader[89], &header_struct.temperature, 4);
	memcpy(&ptr_fileheader[4], header_struct.site_name, 5);
	memcpy(&ptr_fileheader[19], header_struct.filename, 12);
	memcpy(&ptr_fileheader[33], header_struct.month, 3);
	memcpy(&ptr_fileheader[38], header_struct.filter, 6);
}

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
		if (strcmp(parameter_str, "CAMDAEMON_SITE") == 0){
			config_obj.config_site = (char *)malloc(strlen(val_str));
			strcpy(config_obj.config_site, val_str);
			SITENAME = config_obj.config_site;
		} else if (strcmp(parameter_str, "CAMDAEMON_CAM") == 0){
			config_obj.config_cam = (char *)malloc(strlen(val_str));
			strcpy(config_obj.config_cam, val_str);
			FILTER = config_obj.config_cam;
		} else if (strcmp(parameter_str, "CAMDAEMON_PATH") == 0){
			config_obj.config_path = (char *)malloc(strlen(val_str));
			strcpy(config_obj.config_path, val_str);
			LOCATION = config_obj.config_path;
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

	char * filename_ext = (char *)malloc(14);
	strftime(filename_ext,14,"L%H%M%SA.%j", time_info);

	file_path_obj.filename = filename_ext;

	char * folder_name = (char *)malloc(9);
	strftime(folder_name,9,"%b%d%y/", time_info);

	char * folder_path = (char *)malloc( strlen(LOCATION) + strlen(folder_name));
	sprintf(folder_path, "%s%s", LOCATION, folder_name);
	file_path_obj.folder_path = folder_path;

	char * full_path = (char *)malloc( strlen(LOCATION) + strlen(folder_name) + strlen(filename_ext) + 1);
	sprintf(full_path, "%s%s%s", LOCATION, folder_name, filename_ext);
	file_path_obj.full_path = full_path;

	DAY = time_info->tm_mday;
	YEAR = 1900 + time_info->tm_year;
	char * month_name = (char *)malloc(4);
	strftime(month_name,4,"%^b", time_info);
	MONTH = month_name;
	FILENAME = filename_ext;

	// freeing any of these give an error
	// free(time_info);
	// free(fileNamePtr);
	// free(filename_ext);

	return file_path_obj;
}
