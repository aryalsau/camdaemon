#include "../common.c"


// external interfaces
extern int init_camera();
extern int uninit_camera();
extern char * capture(long exp_time);
extern char * preview(long exp_time_ms, int sock);

extern int init_camera(){

	syslog(LOG_INFO, "Initialising camdaemon...\n");

	return 1;
}

extern int uninit_camera(){
    return 0;
}

extern char * capture(long exp_time_ms){

	struct data data_object;

	data_object.xdim = 32;
	data_object.ydim = 32;
	data_object.xbin = 1;
	data_object.ybin = 1;
	data_object.exp_time_ms = exp_time_ms;
	data_object.temperature = -60.0;

	data_object.image = dyanmically_allocate(data_object.xdim,data_object.xdim,sizeof(short*));

	short ii, jj;
	for (jj = 0; jj < data_object.xdim; jj++)
		for (ii = 0; ii < data_object.ydim; ii++)
			data_object.image[ii][jj] = ii+jj;

	struct config config_obj = read_config();
	struct file_path file_path_obj = file_path_string();

	data_object.filename = file_path_obj.filename;
	data_object.site = config_obj.site;
	data_object.camera = config_obj.camera;

	sleep(ceil(data_object.exp_time_ms/1000));

	char * location = (char *)malloc(strlen(config_obj.path)+strlen(file_path_obj.folder_name)+2);
	sprintf(location, "%s/%s", config_obj.path, file_path_obj.folder_name);
	return write_file(data_object, location);
}



extern char * preview(long exp_time_ms, int sock){
	// int n;
	//
	// short cam_handle;
	// short num_frames;
	//
	// EXPTIMEMS = exp_time_ms;
	// WAITTIMEMS  = 60000;
	// TEMPERATURE = -60.0;
	//
	// struct config config_obj = read_config();
	// struct file_path file_path_obj = file_path_string();
	//
	// struct stat st = {0};
	// if (stat(file_path_obj.folder_path, &st) == -1) mkdir(file_path_obj.folder_path, 0700);
	//
	// FILE *data;
	// data = fopen( file_path_obj.full_path , "w" );
	// if( !data ) {
	// 	syslog(LOG_INFO, "%s not opened error\n", file_path_obj.full_path);
	// 	return 0;
	// }
	//
	// struct header image_header = build_header();
	//
	// // syslog(LOG_INFO, "exp_time %f\n", image_header.exp_time);
	// // syslog(LOG_INFO, "day %d\n", image_header.day);
	// // syslog(LOG_INFO, "month %s\n", image_header.month);
	// // syslog(LOG_INFO, "year %d\n", image_header.year);
	// // syslog(LOG_INFO, "filename %s\n", image_header.filename);
	// // syslog(LOG_INFO, "siteName %s\n", image_header.siteName);
	// // syslog(LOG_INFO, "filter %s\n", image_header.filter);
	// // syslog(LOG_INFO, "LOCATION %s\n", LOCATION);
	//
	// unsigned char * header;
	// const int header_size = 128;
	// header = (unsigned char*)malloc(header_size);
	// copy_header(image_header,header);
	// fwrite( header, 1, header_size, data);
	//
	// n = write(sock,header,header_size);
	// if (n < 0) {
	// 	perror("ERROR writing to socket");
	// 	exit(1);
	// }
	//
	// sleep(ceil(image_header.exp_time));
	//
	// const unsigned long size = 1024*1024;
	// unsigned short * frame;
	// frame = (unsigned short*)malloc(size);
	//
	// fwrite( frame, sizeof(unsigned short), 2*size/sizeof(unsigned short), data );
	//
	// syslog(LOG_INFO, "%s created\n", file_path_obj.full_path);
	//
	// n = write(sock,frame,2*size/sizeof(unsigned short));
	// if (n < 0) {
	// 	perror("ERROR writing to socket");
	// 	exit(1);
	// }
	//
	// fclose( data );
	// if( header ) free( header );
	// if( frame ) free( frame );

	return "asdf";
}
