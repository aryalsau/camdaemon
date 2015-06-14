#include "common.c"


// external interfaces
extern int initCamera();
extern char * capture(long expTime);
extern char * preview(long expTimems, int sock);

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

extern int initCamera(){
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
	// siteName = getenv("HITANDMIS_SITE");
	// camName = getenv("HITANDMIS_CAM");
	// path = getenv("HITANDMIS_PATH");
	// if(siteName)
	// 	SITENAME = siteName;
	// if(camName)
	// 	FILTER = camName;
	// if(path)
	// 	LOCATION = path;

	syslog(LOG_INFO, "Initialising %s...\n",camName);
	
	return 1;
}

extern int uninitCamera(){
    return 0;
}

extern char * capture(long expTimems){
	int16 hCam;
	int16 num_frames;

	EXPTIMEMS = expTimems;
	WAITTIMEMS  = 60000;
	TEMPERATURE = -60.0;

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

	// syslog(LOG_INFO, "expTime %f\n", imageHeader.expTime);
	// syslog(LOG_INFO, "day %d\n", imageHeader.day);
	// syslog(LOG_INFO, "month %s\n", imageHeader.month);
	// syslog(LOG_INFO, "year %d\n", imageHeader.year);
	// syslog(LOG_INFO, "filename %s\n", imageHeader.filename);
	// syslog(LOG_INFO, "siteName %s\n", imageHeader.siteName);
	// syslog(LOG_INFO, "filter %s\n", imageHeader.filter);
	// syslog(LOG_INFO, "LOCATION %s\n", LOCATION);

	uns8 * ptrHeader;
	const int headerSize = 128;
	ptrHeader = (uns8*)malloc(headerSize);
	copyHeader(imageHeader,ptrHeader);
	fwrite( ptrHeader, 1, headerSize, data);

	sleep(ceil(imageHeader.expTime));

	const uns32 size = 1024*1024;
	uns16 * frame;
	frame = (uns16*)malloc(size);

	fwrite( frame, sizeof(uns16), 2*size/sizeof(uns16), data );

	fclose( data );

	return filePath.fullpathptr;
}



extern char * preview(long expTimems, int sock){
	int n;

	int16 hCam;
	int16 num_frames;

	EXPTIMEMS = expTimems;
	WAITTIMEMS  = 60000;
	TEMPERATURE = -60.0;

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

	// syslog(LOG_INFO, "expTime %f\n", imageHeader.expTime);
	// syslog(LOG_INFO, "day %d\n", imageHeader.day);
	// syslog(LOG_INFO, "month %s\n", imageHeader.month);
	// syslog(LOG_INFO, "year %d\n", imageHeader.year);
	// syslog(LOG_INFO, "filename %s\n", imageHeader.filename);
	// syslog(LOG_INFO, "siteName %s\n", imageHeader.siteName);
	// syslog(LOG_INFO, "filter %s\n", imageHeader.filter);
	// syslog(LOG_INFO, "LOCATION %s\n", LOCATION);

	uns8 * ptrHeader;
	const int headerSize = 128;
	ptrHeader = (uns8*)malloc(headerSize);
	copyHeader(imageHeader,ptrHeader);
	fwrite( ptrHeader, 1, headerSize, data);

	n = write(sock,ptrHeader,headerSize);
	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}

	sleep(ceil(imageHeader.expTime));

	const uns32 size = 1024*1024;
	uns16 * frame;
	frame = (uns16*)malloc(size);

	fwrite( frame, sizeof(uns16), 2*size/sizeof(uns16), data );

	n = write(sock,frame,2*size/sizeof(uns16));
	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}

	fclose( data );

	return filePath.fullpathptr;
}