#include "../common.c"


// external interfaces
extern int init_camera();
extern int uninit_camera();
extern char * capture(long exp_time);
extern char * write_file(struct data data_obj, char *location);
extern char * preview(long exp_time_ms, int sock);

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
long EXP_tIMEMS;
long WAITTIMEMS;
float TEMPERATURE;
char * SITENAME;
char * FILENAME;
char * MONTH;
char * FILTER;

char * LOCATION;

extern int init_camera(){

	syslog(LOG_INFO, "Initialising camdaemon...\n");

	return 1;
}

extern int uninit_camera(){
    return 0;
}

extern char * write_file(struct data data_obj, char *location){
	int status;
	long nelements = data_obj.xdim*data_obj.ydim;
	long naxis = 2;
	long naxes[2] = {data_obj.xdim,data_obj.ydim};   /* image is 300 pixels wide by 200 rows */
	long fpixel = 1;

	struct stat st = {0};
	if (stat(location, &st) == -1) mkdir(location, 0700);

	char * full_path = (char *)malloc( strlen(location) + strlen(data_obj.filename) + 1);
	sprintf(full_path, "%s/%s", location, data_obj.filename);

	fitsfile *fptr;       /* pointer to the FITS file; defined in fitsio.h */

	status = 0;
	fits_create_file(&fptr, full_path, &status);   /* create new file */
	fits_create_img(fptr, SHORT_IMG, naxis, naxes, &status);

	fits_update_key(fptr, TLONG, "EXPOSURE", &data_obj.exp_time_ms, "Total Exposure Time", &status);
	fits_write_key_unit(fptr, "EXPOSURE", "ms", &status);

	fits_update_key(fptr, TFLOAT, "TEMPERATURE", &data_obj.temperature, "Camera Temperature", &status);
	fits_write_key_unit(fptr, "TEMPERATURE", "C", &status);

	fits_update_key(fptr, TSTRING, "SITE", data_obj.site, "Instrument Location", &status);

	fits_update_key(fptr, TSTRING, "CAMERA", data_obj.camera, "Instrument Camera", &status);

	fits_write_date(fptr, &status);

	/* Write the array of integers to the image */
	fits_write_img(fptr, TSHORT, fpixel, nelements, data_obj.image[0], &status);

	fits_close_file(fptr, &status);            /* close the file */

	fits_report_error(stderr, status);  /* print out any error messages */
	return full_path;
}

extern char * capture(long exp_time_ms){

	struct data data_obj;

	data_obj.xdim = 128;
	data_obj.ydim = 128;
	data_obj.xbin = 1;
	data_obj.ybin = 1;
	data_obj.num_subarrays = 1;
	data_obj.left = 0;
	data_obj.right = 1024;
	data_obj.bottom1 = 0;
	data_obj.top1 = 1024;
	data_obj.num_coadds = 1;
	data_obj.exp_time_ms = exp_time_ms;
	data_obj.temperature = -60.0;

	data_obj.image = dyanmically_allocate(data_obj.xdim,data_obj.xdim,sizeof(short*));

	short ii, jj;
	for (jj = 0; jj < data_obj.xdim; jj++)
		for (ii = 0; ii < data_obj.ydim; ii++)
			data_obj.image[jj][ii] = ii + jj;

	struct config config_obj = read_config();
	struct file_path file_path_obj = file_path_string();

	data_obj.filename = file_path_obj.filename;
	data_obj.site = config_obj.site;
	data_obj.camera = config_obj.camera;

	sleep(ceil(data_obj.exp_time_ms/1000));

	char * location = (char *)malloc( strlen(config_obj.path) + strlen(file_path_obj.folder_name) + 2);
	sprintf(location, "%s/%s", config_obj.path, file_path_obj.folder_name);
	return write_file(data_obj, location);
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
