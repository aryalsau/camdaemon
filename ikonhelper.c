#include "atmcdLXd.h"
#include "common.c"
#include <stdio.h>
#include <unistd.h>

// external interfaces
extern int initCamera(int argc, char *argv[]);
extern int capture(long expTime);

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

// extern int initCamera(int argc, char *argv[]){
// 	// will allways do full frame captures
// 	// so these entries never change so why even have them?
// 	syslog(LOG_INFO, "Initialising...\n");
// 	XDIM = 1024;
// 	YDIM = 1024;
// 	XBIN = 1;
// 	YBIN = 1;
// 	NUMSUBARRAYS = 1;
// 	LEFT = 0;
// 	RIGHT = 1024;
// 	TOP1 = 1024;
// 	BOTTOM1 = 0;
// 	NUMCOADDS = 1;
// 	SITENAME = "LAB";
// 	FILTER = "h&m-a";

// 	LOCATION = "/home/kuravih/Dropbox/Exchange/socketDaemoncopy/data/";

// 	return 0;
// }

// extern int capture(long expTimems){
// 	EXPTIMEMS = expTimems;
// 	WAITTIMEMS  = 60000;
// 	TEMPERATURE = -60.0;

// 	char * fullPath = fpathString(123);

// 	FILE *data;
// 	data = fopen( fullPath , "w" );
// 	if( !data ) {
// 		syslog(LOG_INFO, "%s not opened error\n", fullPath);
// 		return 0;
// 	}

// 	struct header imageHeader = buildHeader();

// 	syslog(LOG_INFO, "expTime %f\n", imageHeader.expTime);
// 	syslog(LOG_INFO, "day %d\n", imageHeader.day);
// 	syslog(LOG_INFO, "month %s\n", imageHeader.month);
// 	syslog(LOG_INFO, "year %d\n", imageHeader.year);
// 	syslog(LOG_INFO, "filename %s\n", imageHeader.filename);
// 	syslog(LOG_INFO, "siteName %s\n", imageHeader.siteName);
// 	syslog(LOG_INFO, "filter %s\n", imageHeader.filter);
// 	syslog(LOG_INFO, "LOCATION %s\n", LOCATION);

// 	uns8 * ptrHeader;
// 	const int headerSize = 128;
// 	ptrHeader = (uns8*)malloc(headerSize);
// 	copyHeader(imageHeader,ptrHeader);
// 	fwrite( ptrHeader, 1, headerSize, data);

// 	const uns32 size = 1024*1024;
// 	uns16 * frame;
// 	frame = (uns16*)malloc(size);

// 	fwrite( frame, sizeof(uns16), 2*size/sizeof(uns16), data );

// 	fclose( data );

// 	return 0;
// }




// internal functions
static int selectCamera(int iNumArgs, char *szArgList[]);

int selectCamera (int iNumArgs, char *szArgList[])
{
	if (iNumArgs == 2) {
		at_32 lNumCameras;
		GetAvailableCameras(&lNumCameras);
		int iSelectedCamera = atoi(szArgList[1]);
	
		if (iSelectedCamera < lNumCameras && iSelectedCamera >= 0) {
			at_32 lCameraHandle;
			GetCameraHandle(iSelectedCamera, &lCameraHandle);
			SetCurrentCamera(lCameraHandle);
			return iSelectedCamera;
		}
		else 
			return -1;
	}
	return 0;
}

int XDIM = 0;
int YDIM = 0;
long EXPTIME = 100;

int initCamera(int argc, char *argv[]){
	unsigned long error;
	int width, height;
	if (selectCamera (argc, argv) < 0) {
		syslog(LOG_INFO,"CAMERA SELECTION ERROR");
		return -1;
	}
	//Initialize CCD
	error = Initialize("/usr/local/etc/andor");
	if(error!=DRV_SUCCESS){
		syslog(LOG_INFO,"Initialisation error...exiting");
		return(1);
	}
	sleep(2); //sleep to allow initialization to complete
	CoolerON();
	SetTemperature(-30);
	//Set Read Mode to --Image--
	SetReadMode(4);
	//Set Acquisition mode to --Single scan--
	SetAcquisitionMode(1);
	//Set initial exposure time
	SetExposureTime((float)(EXPTIME/1000));
	//Get Detector dimensions
	GetDetector(&XDIM, &YDIM);
	//Initialize Shutter
	SetShutter(1,0,50,50);
	//Setup Image dimensions
	SetImage(1,1,1,XDIM,1,YDIM);
	return 0;
}

int getTemperature() {
	int iTemp, status;
	status = GetTemperature(&iTemp);
	if(status==DRV_TEMPERATURE_OFF) {
		//syslog( LOG_INFO, "Cooler is OFF, Temperature %d C",iTemp);
	} else if(status==DRV_TEMPERATURE_STABILIZED) {
		//syslog( LOG_INFO, "Cooler Stabilised, Temperature %d C",iTemp);
	} else {
		//syslog( LOG_INFO, "Cooler is ON, Temperature %d C",iTemp);
	}
	return iTemp;
}

int capture(long expTime_ms) {

	EXPTIME = expTime_ms;

    //updateHeader();

	SetExposureTime((float)(EXPTIME/1000));

	StartAcquisition();

	GetDetector(&XDIM, &YDIM);

	int snapstatus;
	at_32 *imageData [ XDIM*YDIM ];

	//Loop until acquisition finished
	GetStatus(&snapstatus);

	while(snapstatus==DRV_ACQUIRING){
		GetStatus(&snapstatus);
	}

	GetAcquiredData(*imageData, XDIM*YDIM);


	//frame = (uns16*)malloc( size *2 );
//	fwrite( ptrHeader, 1, headerSize, data);
//	fwrite( frame, sizeof( uns16 ), size/sizeof(uns16), data );
	
	SaveAsBmp("/home/kuravih/Dropbox/Exchange/socketDaemon/image.bmp", "/home/kuravih/Dropbox/Exchange/socketDaemon/GREY.PAL", 0, 0);

	return 0;
}
