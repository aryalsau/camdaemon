#include "../common.c"

int init_camera();
int uninit_camera();
struct data capture(unsigned long* exp_time);
char * capture_write(unsigned long* exp_time_ms, char* response);

int init_camera(){
	syslog(LOG_ERR, "initialising camera\n");
	if (verbose) printf("initialising camera\n");
	return 1;
}

int uninit_camera(){
	syslog(LOG_ERR, "uninitialising camera\n");
	if (verbose) printf("uninitialising camera\n");
	return 0;
}

struct data capture(unsigned long* exp_time_ms){

	char exp_time_ms_string[21];
	sprintf(exp_time_ms_string,"beep -f 523.2 -l %lu", *exp_time_ms);

	struct data data_object;

	data_object.xdim = 1024;
	data_object.ydim = 1024;
	data_object.xbin = 1;
	data_object.ybin = 1;
	data_object.exp_time_ms = *exp_time_ms;
	data_object.temp_c = -60.0;

	time_t raw_time;
	time(&raw_time);
	data_object.time_info = gmtime(&raw_time);

	data_object.image = dyanmically_allocate(data_object.xdim,data_object.xdim,sizeof(short*));
	short ii, jj;
	for (jj = 0; jj < data_object.xdim; jj++)
		for (ii = 0; ii < data_object.ydim; ii++)
			data_object.image[ii][jj] = ii+jj;

	struct compass_data compass_data_object = get_compass_data();

	int i;
	for (i = 0; i<3; i++) {
		data_object.acceleration[i] = compass_data_object.acceleration[i];
		data_object.magnetic_field[i] = compass_data_object.magnetic_field[i];
	}

	data_object.config_object = read_config();

	//sleep(ceil(data_object.exp_time_ms/1000));
	system(exp_time_ms_string);

	return data_object;
}


char * capture_write(unsigned long* exp_time_ms, char* response) {
	struct data data_object = capture(exp_time_ms);

	struct file_path file_path_object = time_info_to_file_path(data_object.time_info);

	char * location = (char *)malloc(strlen(data_object.config_object.path)+strlen(file_path_object.folder_name)+2);
	sprintf(location, "%s/%s", data_object.config_object.path, file_path_object.folder_name);

	return write_file(data_object, location, file_path_object.filename);
}
