#include "../common.c"


// external interfaces
extern int init_camera();
extern int uninit_camera();
extern struct data capture(long exp_time);
extern char * capture_write(long exp_time);

extern int init_camera(){

	syslog(LOG_INFO, "Initialising camdaemon...\n");

	return 1;
}

extern int uninit_camera(){
    return 0;
}

extern struct data capture(long exp_time_ms){

	struct data data_object;

	data_object.xdim = 1024;
	data_object.ydim = 1024;
	data_object.xbin = 1;
	data_object.ybin = 1;
	data_object.exp_time_ms = exp_time_ms;
	data_object.temp_c = -60.0;

	time_t raw_time;
	time(&raw_time);
	data_object.time_info = gmtime(&raw_time);

	data_object.image = dyanmically_allocate(data_object.xdim,data_object.xdim,sizeof(short*));
	short ii, jj;
	for (jj = 0; jj < data_object.xdim; jj++)
		for (ii = 0; ii < data_object.ydim; ii++)
			data_object.image[ii][jj] = ii+jj;

	int i;
	for (i = 0; i<3; i++) {
		data_object.acceleration[i] = (double)1/(i+1);
		data_object.field[i] = (double)3/(i+1);
	}

	data_object.config_object = read_config();

	sleep(ceil(data_object.exp_time_ms/1000));

	return data_object;
}


extern char * capture_write(long exp_time_ms) {
	struct data data_object = capture(exp_time_ms);

	struct file_path file_path_object = time_info_to_file_path(data_object.time_info);

	char * location = (char *)malloc(strlen(data_object.config_object.path)+strlen(file_path_object.folder_name)+2);
	sprintf(location, "%s/%s", data_object.config_object.path, file_path_object.folder_name);

	return write_file(data_object, location, file_path_object.filename);
}
