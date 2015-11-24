#include "atmcdLXd.h"
#include "../common.c"
#include <stdio.h>
#include <unistd.h>

// external interfaces
extern int init_camera();
extern int uninit_camera();
extern char * capture(long exp_time_ms);
extern char * preview(long exp_time_ms, int socket);

// internal function
int select_camera (int num_args, char *arg_list[]);
int get_temperature();

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

int select_camera (int num_args, char *arg_list[]) {
	if (num_args == 2) {
		at_32 num_cameras;
		GetAvailableCameras(&num_cameras);
		int selected_camera = atoi(arg_list[1]);

		if (selected_camera < num_cameras && selected_camera >= 0) {
			at_32 camera_handle;
			GetCameraHandle(selected_camera, &camera_handle);
			SetCurrentCamera(camera_handle);
			return selected_camera;
		} else
			return -1;
	}
	return 0;
}

int init_camera() {
	XDIM = 1024;
	YDIM = 1024;
	XBIN = 1;
	YBIN = 1;
	NUMSUBARRAYS = 1;
	LEFT = 0;
	RIGHT = 1024;
	TOP1 = 1024;
	BOTTOM1 = 0;
	NUMCOADDS = 1;
	SITENAME = "NA";
	FILTER = "NA";
	LOCATION = "/media/backup/data/";

	unsigned long error;
	int width, height;

	// if (select_camera (argc, argv) < 0) {
	// 		syslog(LOG_INFO,"CAMERA SELECTION ERROR");
	// 		return -1;
	// }
	//Initialize CCD
	error = Initialize("/usr/local/etc/andor");
	if(error!=DRV_SUCCESS){
			syslog(LOG_INFO,"Initialisation error...exiting");
			return(1);
	}
	sleep(2); //sleep to allow initialization to complete
	CoolerON();
	SetTemperature(-50);
	//Set Read Mode to --Image--
	SetReadMode(4);
	//Set Acquisition mode to --Single scan--
	SetAcquisitionMode(1);
	//Set initial exposure time
	SetExposureTime(0.0100);
	//Get Detector dimensions
	GetDetector(&XDIM, &YDIM);
	//Initialize Shutter
	SetShutter(1,0,50,50);
	//Setup Image dimensions
	SetImage(1,1,1,XDIM,1,YDIM);
	return 0;
}

extern int uninit_camera(){
	ShutDown();
}

int get_temperature() {
	int temperature, status;
	status = GetTemperature(&temperature);
	if(status==DRV_TEMPERATURE_OFF) {
			//syslog( LOG_INFO, "Cooler is OFF, Temperature %d C",temperature);
	} else if(status==DRV_TEMPERATURE_STABILIZED) {
			//syslog( LOG_INFO, "Cooler Stabilised, Temperature %d C",temperature);
	} else {
			//syslog( LOG_INFO, "Cooler is ON, Temperature %d C",temperature);
	}
	return temperature;
}

extern char * capture(long exp_time_ms){

	short temperature;

	temperature = get_temperature();

	syslog(LOG_INFO, "Temperature is %f c\n", (float)temperature);

	EXPTIMEMS = exp_time_ms;
	WAITTIMEMS  = 60000;
	TEMPERATURE = (float)temperature;

	struct config config_obj = read_config();
	struct file_path file_path_obj = file_path_string();

	struct stat st = {0};
	if (stat(file_path_obj.folder_path, &st) == -1) {
			mkdir(file_path_obj.folder_path, 0700);
	}

	FILE *data;
	data = fopen( file_path_obj.full_path , "w" );
	if( !data ) {
			syslog(LOG_INFO, "%s not opened error\n", file_path_obj.full_path);
			return 0;
	}

	struct header image_header = build_header();

	unsigned char * header;
	const int header_size = 128;
	header = (unsigned char *)malloc(header_size);
	copy_header(image_header,header);
	fwrite( header, 1, header_size, data);

	sleep(ceil((float)exp_time_ms/1000));

	SetExposureTime((float)(EXPTIMEMS/1000));

	StartAcquisition();

	GetDetector(&XDIM, &YDIM);

	const unsigned long size = XDIM*YDIM;
	unsigned short * frame;
	frame = (unsigned short*)malloc( size *2 );

	int snap_status;
	//Loop until acquisition finished
	GetStatus(&snap_status);

	while(snap_status==DRV_ACQUIRING) {
			GetStatus(&snap_status);
	}

	GetAcquiredData16(frame, (unsigned long)size);

	//SaveAsBmp("/home/kuravih/Documents/image.bmp", "/home/kuravih/Dropbox/Exchange/socketDaemon/GREY.PAL", 0, 0);

	fwrite( frame, sizeof(unsigned short), (size_t)size, data );

	syslog(LOG_INFO, "%s created\n", file_path_obj.full_path);

	fclose( data );
	if( header ) free( header );
	if( frame ) free( frame );

	return file_path_obj.full_path;
}


extern char * preview(long exp_time_ms, int socket){
	int n;

	short temperature;

	temperature = get_temperature();

	syslog(LOG_INFO, "Temperature is %f c\n", (float)temperature);

	EXPTIMEMS = exp_time_ms;
	WAITTIMEMS  = 60000;
	TEMPERATURE = (float)temperature;

	struct config config_obj = read_config();
	struct file_path file_path_obj = file_path_string();

	struct stat st = {0};
	if (stat(file_path_obj.folder_path, &st) == -1) {
			mkdir(file_path_obj.folder_path, 0700);
	}

	FILE *data;
	data = fopen( file_path_obj.full_path , "w" );
	if( !data ) {
			syslog(LOG_INFO, "%s not opened error\n", file_path_obj.full_path);
			return 0;
	}

	struct header image_header = build_header();

	unsigned char * header;
	const int header_size = 128;
	header = (unsigned char *)malloc(header_size);
	copy_header(image_header,header);
	fwrite( header, 1, header_size, data);

	sleep(ceil((float)exp_time_ms/1000));

	SetExposureTime((float)(EXPTIMEMS/1000));

	StartAcquisition();

	GetDetector(&XDIM, &YDIM);

	const unsigned long size = XDIM*YDIM;
	unsigned short * frame;
	frame = (unsigned short*)malloc( size *2 );

	int snap_status;
	//Loop until acquisition finished
	GetStatus(&snap_status);

	while(snap_status==DRV_ACQUIRING) {
			GetStatus(&snap_status);
	}

	GetAcquiredData16(frame, (unsigned long)size);

	//SaveAsBmp("/home/kuravih/Documents/image.bmp", "/home/kuravih/Dropbox/Exchange/socketDaemon/GREY.PAL", 0, 0);

	fwrite( frame, sizeof(unsigned short), (size_t)size, data );

	syslog(LOG_INFO, "%s created\n", file_path_obj.full_path);

	n = write(socket,header,header_size);
	if (n < 0) {
			perror("ERROR writing to socket");
			exit(1);
	}

	n = write(socket,frame,2*size);
	if (n < 0) {
			perror("ERROR writing to socket");
			exit(1);
	}

	fclose( data );
	if( header ) free( header );
	if( frame ) free( frame );

	return file_path_obj.full_path;
}
