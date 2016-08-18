#include <syslog.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include "../common.h"
#include "../camera.h"
#include "atmcdLXd.h"

// int CameraSelect (int iNumArgs, char* szArgList[]);
//
// int CameraSelect (int iNumArgs, char* szArgList[]) {
// 	if (iNumArgs == 2) {
// 		at_32 lNumCameras;
// 		GetAvailableCameras(&lNumCameras);
// 		int iSelectedCamera = atoi(szArgList[1]);
// 		if (iSelectedCamera < lNumCameras && iSelectedCamera >= 0) {
// 		  at_32 lCameraHandle;
// 		  GetCameraHandle(iSelectedCamera, &lCameraHandle);
// 		  SetCurrentCamera(lCameraHandle);
// 		  return iSelectedCamera;
// 		} else {
// 			return -1;
// 		}
// 	}
// 	return 0;
// }


int init_camera(void){
	unsigned long error;

	// if (CameraSelect (argc, argv) < 0) {
	// 	syslog(LOG_ERR, "ikon initialization failed\n");
	// 	if (VERBOSE) printf("ikon initialization failed\n");
	// 	exit(EXIT_FAILURE)
	// }else{
	// 	syslog(LOG_INFO, "initialising ikon camera...\n");
	// 	if (VERBOSE) printf("initialising ikon camera...\n");
	// 	return 0;
	// }

	error = Initialize("/usr/local/etc/andor");
	if(error!=DRV_SUCCESS){
			syslog(LOG_ERR,"initialisation error...exiting");
			if (VERBOSE) printf("initialisation error...exiting");
			exit(EXIT_FAILURE);
	}

	sleep(2); //sleep to allow initialization to complete
	CoolerON();
	SetTemperature(-50);
	SetReadMode(4); //Set Read Mode to --Image--
	SetAcquisitionMode(1); //Set Acquisition mode to --Single scan--

	return 0;
}


int uninit_camera(void){
	ShutDown();
	syslog(LOG_INFO,"uninitialising ikon camera\n");
	if (VERBOSE) printf("uninitialising ikon camera\n");
	return 0;
}


int acquire_camera_imagedata(struct Data* data) {
	int snap_status;
	unsigned long size;

	SetExposureTime((float)(data->exp_time_us/1000000));
	GetDetector((int*)&(data->xdim), (int*)&(data->ydim));
	size = data->xdim * data->ydim;
	allocate_frame(&(data->imagedata), (int)data->xdim, (int)data->ydim);

	StartAcquisition();
	GetStatus(&snap_status);
	while(snap_status==DRV_ACQUIRING) {
			GetStatus(&snap_status);
	}

	GetAcquiredData16(data->imagedata, (unsigned long)size);
	return 0;
}


int acquire_camera_temp(struct Data* data) {
	int temperature;
	syslog(LOG_INFO,"retrieving camera temperature\n");
	if (VERBOSE) printf("retrieving camera temperature\n");
	GetTemperature(&temperature);
	data->temp_c = (float)temperature/100;
	return 0;
}
