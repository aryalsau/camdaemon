#include <syslog.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "../common.h"
#include "../camera.h"
#include "master.h"
#include "pvcam.h"

int16 camera_handle;

int print_pv_error(void);
int set_any_param(int16 hCam, uns32 param_id, void* param_value);
int get_any_param(int16 hCam, uns32 param_id, void* param_value);


int print_pv_error() {
	char msg[ERROR_MSG_LEN];
	int16 i = pl_error_code();
	pl_error_message(i,msg);
	syslog(LOG_ERR," [%i]%s\n", i, msg );
	return 0;
}


int set_any_param(int16 hCam, uns32 param_id, void *param_value) {
	rs_bool b_status, b_param;
	uns16 param_access;

	b_status = pl_get_param( hCam, param_id, ATTR_AVAIL, (void*)&b_param );
	if( b_param ) {
		b_status = pl_get_param( hCam, param_id, ATTR_ACCESS, (void*)&param_access );
		if( param_access == ACC_READ_WRITE || param_access == ACC_WRITE_ONLY ) {
			if( !pl_set_param( hCam, param_id, param_value) ) {
				syslog(LOG_ERR,"error: param did not get set\n" );
				if (VERBOSE) printf("error: param did not get set\n" );
			}
		}else {
			syslog(LOG_ERR,"error: param is not writable\n" );
			if (VERBOSE) printf("error: param is not writable\n" );
		}
	} else {
		syslog(LOG_ERR,"error: param is not available\n" );
		if (VERBOSE) printf("error: param is not available\n" );
	}
	return 0;
}


int get_any_param(int16 hCam, uns32 param_id, void *param_value) {
	rs_bool b_status, b_param;
	uns16 param_access;

	b_status = pl_get_param( hCam, param_id, ATTR_AVAIL, (void*)&b_param );
	if( b_param ) {
		b_status = pl_get_param( hCam, param_id, ATTR_ACCESS, (void*)&param_access );
		if( param_access == ACC_READ_WRITE || param_access == ACC_READ_ONLY ) {
			if( !pl_get_param( hCam, param_id, ATTR_CURRENT, param_value ) ) {
				syslog(LOG_ERR,"error: param couldnt be not be read\n" );
				if (VERBOSE) printf("error: param couldnt be not be read\n" );
			}
		}else {
			syslog(LOG_ERR,"error: param is not readable\n" );
			if (VERBOSE) printf("error: param is not readable\n" );
		}
	} else {
		syslog(LOG_ERR,"error: param is not available\n" );
		if (VERBOSE) printf("error: param is not available\n" );
	}
	return 0;
}


int init_camera(void){

	int camera_selection = 0;
	char camera_name[CAM_NAME_LEN];
	int16 set_temp = -5000;

	if(!pl_pvcam_init()) {
		print_pv_error();
		syslog(LOG_ERR,"failed to init pvcam...exitting\n");
		if (VERBOSE) printf("failed to init pvcam...exitting\n");
		exit(EXIT_FAILURE);
	} else {
		syslog(LOG_INFO,"pvcam lib initialized\n");
		if (VERBOSE) printf("pvcam lib initialized\n");
	}

	if(pl_cam_get_name(camera_selection, camera_name)) {
		syslog(LOG_INFO,"camera 0 name is %s\n", camera_name);
		if (VERBOSE) printf("camera 0 name is %s\n", camera_name);
	} else {
		print_pv_error();
		syslog(LOG_ERR,"could not get camera name...exitting\n");
		if (VERBOSE) printf("could not get camera name...exitting\n");
		exit(EXIT_FAILURE);
	}

	if(pl_cam_open(camera_name, &camera_handle, OPEN_EXCLUSIVE)) {
		syslog(LOG_INFO,"camera %s open\n", camera_name);
		if (VERBOSE) printf("camera %s open\n", camera_name);
	} else {
		print_pv_error();
		syslog(LOG_ERR,"camera %s could not be opened...exitting\n", camera_name );
		if (VERBOSE) printf("camera %s could not be opened...exitting\n", camera_name );
		exit(EXIT_FAILURE);
	}

	set_any_param(camera_handle, PARAM_TEMP_SETPOINT, &set_temp );
	syslog(LOG_INFO,"initialising pixis camera...\n");
	if (VERBOSE) printf("initialising pixis camera...\n");

	return 1;
}


int uninit_camera(void){
	syslog(LOG_INFO,"uninitialising pixis camera\n");
	if (VERBOSE) printf("uninitialising pixis camera\n");
	pl_cam_close(camera_handle);
	pl_pvcam_uninit();
	return 0;
}


int acquire_camera_imagedata(struct Data* data) {

	int16 status;
	uns32 not_needed;
	rgn_type region;
	unsigned long size;

	pl_get_param(camera_handle, PARAM_SER_SIZE, ATTR_DEFAULT, (void*)&(data->xdim));
	pl_get_param(camera_handle, PARAM_PAR_SIZE, ATTR_DEFAULT, (void*)&(data->ydim));

	size = (data->xdim/data->xbin)*(data->ydim/data->ybin)*2;

	region.s1 = 0;
	region.p1 = 0;
	region.s2 = data->xdim-1;
	region.p2 = data->ydim - 1;
	region.sbin = data->xbin;
	region.pbin = data->ybin;

	allocate_frame(&(data->imagedata), data->xdim, data->ydim);

	if( pl_exp_init_seq() ) {
		syslog(LOG_INFO,"experiment sequence initialized\n" );
		if (VERBOSE) printf("experiment sequence initialized\n" );
	} else {
		print_pv_error();
		syslog(LOG_ERR,"experiment sequence initialization failed...exitting\n" );
		if (VERBOSE) printf("experiment sequence initialization failed...exitting\n" );
		exit(EXIT_FAILURE);
	}

	if( pl_exp_setup_seq(camera_handle, 1, 1, &region, TIMED_MODE, data->exp_time_us, &size)) {
		syslog(LOG_INFO,"sequence setup with frame size = %lu\n",size);
		if (VERBOSE) printf("sequence setup with frame size = %lu\n",size);
	} else {
		print_pv_error();
		syslog(LOG_ERR,"sequence setup failed...exitting\n" );
		if (VERBOSE) printf("sequence setup failed...exitting\n" );
		exit(EXIT_FAILURE);
	}

	pl_exp_start_seq(camera_handle, &(data->imagedata));

	while( pl_exp_check_status(camera_handle, &status, &not_needed) && (status != READOUT_COMPLETE && status != READOUT_FAILED) );

	if( status == READOUT_FAILED ) {
		print_pv_error();
		syslog(LOG_ERR,"readout error: %i\n", pl_error_code());
		if (VERBOSE) printf("readout error: %i\n", pl_error_code());
		exit(EXIT_FAILURE);
	}

	pl_exp_finish_seq(camera_handle, &(data->imagedata), 0);
	pl_exp_uninit_seq();
	pl_exp_stop_cont(camera_handle,0);
	return 0;
}


int acquire_camera_temp(struct Data* data) {
	int16 temperature;

	syslog(LOG_ERR,"retrieving camera temperature\n");
	if (VERBOSE) printf("retrieving camera temperature\n");

	get_any_param(camera_handle, PARAM_TEMP, &temperature);

	data->temp_c = (float)temperature/100;
	return 0;
}
