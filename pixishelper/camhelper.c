#include "master.h"
#include "../common.c"
#include "pvcam.h"
//sudo gcc -o camdaemon main.c -lpvcam -lm -ldl -lpthread -lraw1394 -I/usr/local/pvcam/examples
// external interfaces
extern int init_camera();
extern int uninit_camera();
extern char * capture(long exp_time_);
extern char * preview(long exp_time_ms, int sock);

// internal function
static void set_ROI( rgn_type* roi, uns16 s1, uns16 s2, uns16 sbin, uns16 p1, uns16 p2, uns16 pbin );
static void print_ROI( int16 roi_count, rgn_type* roi );
static void set_full_frame( int16 hcam, rgn_type* roi );
static void print_pv_error( );
static void set_any_param( int16 hCam, uns32 param_id, void *param_value );
static void get_any_param( int16 hCam, uns32 param_id, void *param_value );
static void set_ADC( int16 hcam, uns32 port, int16 adc_index, int16 gain );
static void acquire_standard( int16 hCam, uns16 * frameBuffer);

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

int16 camera_handle;

static void set_ROI( rgn_type* roi, uns16 s1, uns16 s2, uns16 sbin, uns16 p1, uns16 p2, uns16 pbin ) {
	roi->s1 = s1;
	roi->s2 = s2;
	roi->sbin = sbin;
	roi->p1 = p1;
	roi->p2 = p2;
	roi->pbin = pbin;
}

static void print_ROI( int16 roi_count, rgn_type* roi ) {
	int i;

	for( i = 0; i < roi_count; i++ ) {
		syslog(LOG_INFO, "ROI %i set to { %i, %i, %i, %i, %i, %i }\n", i, roi[i].s1, roi[i].s2, roi[i].sbin, roi[i].p1, roi[i].p2, roi[i].pbin );
	}
}

static void set_full_frame( int16 hcam, rgn_type* roi ) {
	uns16 param;

	roi->s1 = 0;
	pl_get_param( hcam, PARAM_SER_SIZE, ATTR_DEFAULT, (void *)&param );
	roi->s2 = param-1;
	roi->sbin = 1;
	roi->p1 = 0;
	pl_get_param( hcam, PARAM_PAR_SIZE, ATTR_DEFAULT, (void *)&param );
	roi->p2 = param-1;
	roi->pbin = 1;
}

static void print_pv_error( ) {
	char msg[ERROR_MSG_LEN];
	int16 i;

	i = pl_error_code();
	pl_error_message( i, msg );
	syslog(LOG_INFO, " [%i]%s\n", i, msg );
}

static void set_any_param( int16 hCam, uns32 param_id, void *param_value ) {
	rs_bool b_status, b_param;
	uns16 param_access;

	b_status = pl_get_param( hCam, param_id, ATTR_AVAIL, (void*)&b_param );
	if( b_param ) {
		b_status = pl_get_param( hCam, param_id, ATTR_ACCESS, (void*)&param_access );
		if( param_access == ACC_READ_WRITE || param_access == ACC_WRITE_ONLY ) {
			if( !pl_set_param( hCam, param_id, param_value) ) {
				syslog(LOG_INFO, "error: param did not get set\n" );
			}
		}else {
			syslog(LOG_INFO, "error: param is not writable\n" );
		}
	} else {
		syslog(LOG_INFO, "error: param is not available\n" );
	}
}

static void get_any_param( int16 hCam, uns32 param_id, void *param_value ) {
	rs_bool b_status, b_param;
	uns16 param_access;

	b_status = pl_get_param( hCam, param_id, ATTR_AVAIL, (void*)&b_param );
	if( b_param ) {
		b_status = pl_get_param( hCam, param_id, ATTR_ACCESS, (void*)&param_access );
		if( param_access == ACC_READ_WRITE || param_access == ACC_READ_ONLY ) {
			if( !pl_get_param( hCam, param_id, ATTR_CURRENT, param_value ) ) {
				syslog(LOG_INFO, "error: param couldnt be not be read\n" );
			}
		}else {
			syslog(LOG_INFO, "error: param is not readable\n" );
		}
	} else {
		syslog(LOG_INFO, "error: param is not available\n" );
	}
}

static void set_ADC( int16 hcam, uns32 port, int16 adc_index, int16 gain ) {
    set_any_param( hcam, PARAM_READOUT_PORT, &port );
    set_any_param( hcam, PARAM_SPDTAB_INDEX, &adc_index );
    set_any_param( hcam, PARAM_GAIN_INDEX, &gain );
}












extern int init_camera(){
	// will allways do full frame captures
	// so these entries never change so why even have them?

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

	// char *siteName, *camName, *path;
	// siteName = getenv("CAMDAEMON_SITE");
	// camName = getenv("CAMDAEMON_CAM");
	// path = getenv("CAMDAEMON_PATH");
	// if(siteName)
	//     SITENAME = siteName;
	// if(camName)
	//     FILTER = camName;
	// if(path)
	//     LOCATION = path;

	int16 cam_selection = 0;
	char cam_name[CAM_NAME_LEN];

	/* Initialize the PVCam Library and Open the Camera */
	if( !pl_pvcam_init() ) {
		syslog(LOG_INFO, "failed to init pvcam\n" );
		print_pv_error();
		exit( 0 );
	} else {
		syslog(LOG_INFO, "pvcam lib initialized\n" );
	}

	if( pl_cam_get_name( cam_selection, cam_name ) ) {
		syslog(LOG_INFO, "camname for cam 0 is %s\n", cam_name );
	} else {
		syslog(LOG_INFO, "didn't get cam name\n" );
		print_pv_error();
		exit( 0 );
	}

	if( pl_cam_open(cam_name, &camera_handle, OPEN_EXCLUSIVE ) ) {
		syslog(LOG_INFO, "camera %s open\n", cam_name );
	} else {
		syslog(LOG_INFO, "camera %s didn't open\n", cam_name );
		print_pv_error();
		exit( 0 );
	}

	int16 set_temp = -5000;
	set_any_param(camera_handle, PARAM_TEMP_SETPOINT, &set_temp );
	syslog(LOG_INFO, "Initialising camdaemon...\n");

	return 1;
}

extern int uninit_camera(){
	pl_cam_close(camera_handle);
	pl_pvcam_uninit();
	return 0;
}

extern char * capture(long exp_time_ms){

	int16 num_frames;
	int16 temperature;
	get_any_param(camera_handle, PARAM_TEMP, &temperature);

	EXPTIMEMS = exp_time_ms;
	WAITTIMEMS  = 60000;
	TEMPERATURE = (float)temperature/100;

	syslog(LOG_INFO, "Temperature is %f c\n", TEMPERATURE);

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

	// syslog(LOG_INFO, "exp_time_ %f\n", image_header.exp_time_);
	// syslog(LOG_INFO, "day %d\n", image_header.day);
	// syslog(LOG_INFO, "month %s\n", image_header.month);
	// syslog(LOG_INFO, "year %d\n", image_header.year);
	// syslog(LOG_INFO, "filename %s\n", image_header.filename);
	// syslog(LOG_INFO, "siteName %s\n", image_header.siteName);
	// syslog(LOG_INFO, "filter %s\n", image_header.filter);
	// syslog(LOG_INFO, "LOCATION %s\n", LOCATION);

	uns8 * header;
	const int header_size = 128;
	header = (uns8*)malloc(header_size);
	copy_header(image_header,header);
	fwrite( header, 1, header_size, data);

	sleep(ceil((float)exp_time_ms/1000));

	const uns32 size = 1024*1024;
	uns16 * frame;
	frame = (uns16*)malloc( size *2 );
	// if( !frame ) {
	//     printf( "memory allocation error!\n" );
	//     return;
	// } else {
	//     printf( "frame of data at address %x\n", frame );
	// }

	acquire_standard(camera_handle,frame);

	fwrite( frame, sizeof(uns16), (size_t)size, data );

	syslog(LOG_INFO, "%s created\n", file_path_obj.full_path);

	fclose( data );
	if( header ) free( header );
	if( frame ) free( frame );

	return file_path_obj.full_path;
}


extern char * preview(long exp_time_ms, int sock){
	int n;

	int16 num_frames;
	int16 temperature;
	get_any_param(camera_handle, PARAM_TEMP, &temperature);

	EXPTIMEMS = exp_time_ms;
	WAITTIMEMS  = 60000;
	TEMPERATURE = (float)temperature/100;

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

	// syslog(LOG_INFO, "exp_time_ %f\n", image_header.exp_time_);
	// syslog(LOG_INFO, "day %d\n", image_header.day);
	// syslog(LOG_INFO, "month %s\n", image_header.month);
	// syslog(LOG_INFO, "year %d\n", image_header.year);
	// syslog(LOG_INFO, "filename %s\n", image_header.filename);
	// syslog(LOG_INFO, "siteName %s\n", image_header.siteName);
	// syslog(LOG_INFO, "filter %s\n", image_header.filter);
	// syslog(LOG_INFO, "LOCATION %s\n", LOCATION);

	uns8 * header;
	const int header_size = 128;
	header = (uns8*)malloc(header_size);
	copy_header(image_header,header);
	fwrite( header, 1, header_size, data);

	n = write(sock,header,header_size);
	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}

	sleep(ceil((float)exp_time_ms/1000));

	const uns32 size = 1024*1024;
	uns16 * frame;
	frame = (uns16*)malloc( size *2 );
	// if( !frame ) {
	//     printf( "memory allocation error!\n" );
	//     return;
	// } else {
	//     printf( "frame of data at address %x\n", frame );
	// }

	acquire_standard(camera_handle,frame);

	fwrite( frame, sizeof(uns16), (size_t)size, data );

	syslog(LOG_INFO, "%s created\n", file_path_obj.full_path);

	n = write(sock,header,header_size);
	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}

	n = write(sock,frame,2*size);
	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}

	fclose( data );
	if( header ) free( header );
	if( frame ) free( frame );

	return file_path_obj.full_path;
}




static void acquire_standard( int16 hCam, uns16 * frameBuffer){

	rgn_type *region;
	uns32 size;
	int16 status;
	uns32 not_needed;
	uns32 exp_time = EXPTIMEMS;

	region = malloc( sizeof( rgn_type ) );
	set_full_frame( hCam, region );

	/* Init a sequence set the region, exposure mode and exposure time */
	if( pl_exp_init_seq() ) {
		syslog(LOG_INFO, "experiment sequence initialized\n" );
	} else {
		syslog(LOG_INFO, "init_seq failed!\n" );
		return;
	}

	if( pl_exp_setup_seq( hCam, 1, 1, region, TIMED_MODE, (uns32)EXPTIMEMS, &size ) ) {
		syslog(LOG_INFO, "setup sequence OK\n" );
		syslog(LOG_INFO, "frame size = %lu\n", size );
	} else {
		syslog(LOG_INFO, "experiment setup failed!\n" );
		return;
	}

	/* Start the acquisition */

	/* ACQUISITION LOOP */

	pl_exp_start_seq( hCam, frameBuffer );

	/* wait for data or error */
	while( pl_exp_check_status( hCam, &status, &not_needed ) && (status != READOUT_COMPLETE && status != READOUT_FAILED) );

	/* Check Error Codes */
	if( status == READOUT_FAILED ) {
		syslog(LOG_INFO, "Data collection error: %i\n", pl_error_code() );
	}



	/* Finish the sequence */
	pl_exp_finish_seq( hCam, frameBuffer, 0);

	/*Uninit the sequence */
	pl_exp_uninit_seq();

	if( region ) free( region );

}
