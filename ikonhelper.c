#include "atmcdLXd.h"
#include "common.c"
#include <stdio.h>
#include <unistd.h>

// external interfaces
extern int initCamera();
extern int uninitCamera();
extern char * capture(long expTimems);
extern char * preview(long expTimems, int sock);

// internal function
int selectCamera (int iNumArgs, char *szArgList[]);
int getTemperature();

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

int selectCamera (int iNumArgs, char *szArgList[]) {
		if (iNumArgs == 2) {
				at_32 lNumCameras;
				GetAvailableCameras(&lNumCameras);
				int iSelectedCamera = atoi(szArgList[1]);

				if (iSelectedCamera < lNumCameras && iSelectedCamera >= 0) {
						at_32 lCameraHandle;
						GetCameraHandle(iSelectedCamera, &lCameraHandle);
						SetCurrentCamera(lCameraHandle);
						return iSelectedCamera;
				} else
						return -1;
		}
		return 0;
}

int initCamera() {
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

		// if (selectCamera (argc, argv) < 0) {
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

extern int uninitCamera(){
	ShutDown();
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

extern char * capture(long expTimems){

		int16 getTemp;

		getTemp = getTemperature();

		syslog(LOG_INFO, "Temperature is %f c\n", (float)getTemp);

		EXPTIMEMS = expTimems;
		WAITTIMEMS  = 60000;
		TEMPERATURE = (float)getTemp/100;

		struct config configObj = readConfig();
		struct filepath filePath = filePathString();

		struct stat st = {0};
		if (stat(filePath.folderpathptr, &st) == -1) {
				mkdir(filePath.folderpathptr, 0700);
		}

		FILE *data;
		data = fopen( filePath.fullpathptr , "w" );
		if( !data ) {
				syslog(LOG_INFO, "%s not opened error\n", filePath.fullpathptr);
				return 0;
		}

		struct header imageHeader = buildHeader();

		uns8 * ptrHeader;
    const int headerSize = 128;
    ptrHeader = (uns8*)malloc(headerSize);
    copyHeader(imageHeader,ptrHeader);
    fwrite( ptrHeader, 1, headerSize, data);

    sleep(ceil((float)expTimems/1000));

    const uns32 size = 1024*1024;
    uns16 * frame;
    frame = (uns16*)malloc( size *2 );

		SetExposureTime((float)(EXPTIMEMS/1000));

		StartAcquisition();

		GetDetector(&XDIM, &YDIM);

		const uns32 size = XDIM*YDIM;
		unsigned short * frame;
    frame = (unsigned short*)malloc( size *2 );

		int snapstatus;
		at_32 *imageData [ XDIM*YDIM ];

		//Loop until acquisition finished
		GetStatus(&snapstatus);

		while(snapstatus==DRV_ACQUIRING) {
				GetStatus(&snapstatus);
		}

		GetAcquiredData16(frame, (unsigned long)size);

		//SaveAsBmp("/home/kuravih/Documents/image.bmp", "/home/kuravih/Dropbox/Exchange/socketDaemon/GREY.PAL", 0, 0);

		fwrite( frame, sizeof(uns16), (size_t)size, data );

		syslog(LOG_INFO, "%s created\n", filePath.fullpathptr);

		fclose( data );
		if( ptrHeader ) free( ptrHeader );
		if( frame ) free( frame );

		return filePath.fullpathptr;
}


extern char * preview(long expTimems, int sock){
	int n;

	int16 getTemp;

	getTemp = getTemperature();

	syslog(LOG_INFO, "Temperature is %f c\n", (float)getTemp);

	EXPTIMEMS = expTimems;
	WAITTIMEMS  = 60000;
	TEMPERATURE = (float)getTemp/100;

	struct config configObj = readConfig();
	struct filepath filePath = filePathString();

	struct stat st = {0};
	if (stat(filePath.folderpathptr, &st) == -1) {
			mkdir(filePath.folderpathptr, 0700);
	}

	FILE *data;
	data = fopen( filePath.fullpathptr , "w" );
	if( !data ) {
			syslog(LOG_INFO, "%s not opened error\n", filePath.fullpathptr);
			return 0;
	}

	struct header imageHeader = buildHeader();

	uns8 * ptrHeader;
  const int headerSize = 128;
  ptrHeader = (uns8*)malloc(headerSize);
  copyHeader(imageHeader,ptrHeader);
  fwrite( ptrHeader, 1, headerSize, data);

  sleep(ceil((float)expTimems/1000));

    const uns32 size = 1024*1024;
    uns16 * frame;
    frame = (uns16*)malloc( size *2 );

	SetExposureTime((float)(EXPTIMEMS/1000));

	StartAcquisition();

	GetDetector(&XDIM, &YDIM);

	const uns32 size = XDIM*YDIM;
	unsigned short * frame;
	frame = (unsigned short*)malloc( size *2 );

	int snapstatus;
		at_32 *imageData [ XDIM*YDIM ];

	//Loop until acquisition finished
	GetStatus(&snapstatus);

	while(snapstatus==DRV_ACQUIRING) {
			GetStatus(&snapstatus);
	}

	GetAcquiredData16(frame, (unsigned long)size);

	//SaveAsBmp("/home/kuravih/Documents/image.bmp", "/home/kuravih/Dropbox/Exchange/socketDaemon/GREY.PAL", 0, 0);

	fwrite( frame, sizeof(uns16), (size_t)size, data );

	syslog(LOG_INFO, "%s created\n", filePath.fullpathptr);

	n = write(sock,ptrHeader,headerSize);
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
	if( ptrHeader ) free( ptrHeader );
	if( frame ) free( frame );

	return filePath.fullpathptr;
}
