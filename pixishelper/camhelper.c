#include "master.h"
#include "../common.c"
#include "pvcam.h"
//sudo gcc -o camdaemon main.c -lpvcam -lm -ldl -lpthread -lraw1394 -I/usr/local/pvcam/examples
// external interfaces
extern int init_camera();
extern int uninit_camera();
extern struct data capture(long exp_time);
extern char * capture_write(long exp_time);


// internal function
static void set_ROI( rgn_type* roi, uns16 s1, uns16 s2, uns16 sbin, uns16 p1, uns16 p2, uns16 pbin );
static void print_ROI( int16 roi_count, rgn_type* roi );
static void set_full_frame( int16 hcam, rgn_type* roi );
static void print_pv_error( );
static void set_any_param( int16 hCam, uns32 param_id, void *param_value );
static void get_any_param( int16 hCam, uns32 param_id, void *param_value );
static void set_ADC( int16 hcam, uns32 port, int16 adc_index, int16 gain );
static void acquire_standard( int16 hCam, uns16 * frameBuffer, uns32 exp_time);

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




static void acquire_standard( int16 hCam, uns16 * frameBuffer, uns32 exp_time){

	rgn_type *region;
	uns32 size;
	int16 status;
	uns32 not_needed;

	region = malloc( sizeof( rgn_type ) );
	set_full_frame( hCam, region );

	/* Init a sequence set the region, exposure mode and exposure time */
	if( pl_exp_init_seq() ) {
		syslog(LOG_INFO, "experiment sequence initialized\n" );
	} else {
		syslog(LOG_INFO, "init_seq failed!\n" );
		return;
	}

	if( pl_exp_setup_seq( hCam, 1, 1, region, TIMED_MODE, exp_time, &size ) ) {
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


// extern char * capture(long exp_time_ms){
extern struct data capture(long exp_time_ms){

	struct data data_object;

	data_object.xdim = 1024;
	data_object.ydim = 1024;
	data_object.xbin = 1;
	data_object.ybin = 1;
	data_object.exp_time_ms = exp_time_ms;
	// data_object.temp_c = -60.0;

	int16 num_frames;
	int16 temperature;
	get_any_param(camera_handle, PARAM_TEMP, &temperature);
	data_object.temp_c = (float)temperature/100;
	syslog(LOG_INFO, "Temperature is %f c\n", data_object.temp_c);

	time_t raw_time;
	time(&raw_time);
	data_object.time_info = gmtime(&raw_time);

	const uns32 size = 1024*1024;
	uns16 * frame;
	frame = (uns16*)malloc( size *2 );
	if( !frame ) {
	    printf( "memory allocation error!\n" );
	    return;
	} else {
	    // printf( "frame of data at address %i\n", frame );
	}
	acquire_standard(camera_handle,frame,exp_time_ms);

	data_object.image = dyanmically_allocate(data_object.xdim,data_object.xdim,sizeof(short*));
	short ii, jj;
	for (jj = 0; jj < data_object.xdim; jj++)
		for (ii = 0; ii < data_object.ydim; ii++)
			data_object.image[ii][jj] = frame[ii+data_object.xdim*jj];


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
