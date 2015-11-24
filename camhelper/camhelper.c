#include "../common.c"


// external interfaces
extern int init_camera();
extern int uninit_camera();
extern char * capture(long exp_time);
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
	LOCATION = "data/";

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

	syslog(LOG_INFO, "Initialising camdaemon...\n");

	return 1;
}

extern int uninit_camera(){
    return 0;
}

extern char * capture(long exp_time_ms){
	short cam_handle;
	short num_frames;

	EXPTIMEMS = exp_time_ms;
	WAITTIMEMS  = 60000;
	TEMPERATURE = -60.0;

	struct config config_obj = read_config();
	struct file_path file_path_obj = file_path_string();

	struct stat st = {0};
	if (stat(file_path_obj.folder_path, &st) == -1) mkdir(file_path_obj.folder_path, 0700);

	FILE *data;
	data = fopen( file_path_obj.full_path , "w" );
	if( !data ) {
		syslog(LOG_INFO, "%s not opened error\n", file_path_obj.full_path);
		return 0;
	}

	struct header image_header = build_header();

	// syslog(LOG_INFO, "exp_time %f\n", image_header.exp_time);
	// syslog(LOG_INFO, "day %d\n", image_header.day);
	// syslog(LOG_INFO, "month %s\n", image_header.month);
	// syslog(LOG_INFO, "year %d\n", image_header.year);
	// syslog(LOG_INFO, "filename %s\n", image_header.filename);
	// syslog(LOG_INFO, "siteName %s\n", image_header.siteName);
	// syslog(LOG_INFO, "filter %s\n", image_header.filter);
	// syslog(LOG_INFO, "LOCATION %s\n", LOCATION);

	unsigned char * header;
	const int header_size = 128;
	header = (unsigned char*)malloc(header_size);
	copy_header(image_header,header);
	fwrite( header, 1, header_size, data);

	sleep(ceil(image_header.exp_time));

	const unsigned long size = 1024*1024;
	unsigned short * frame;
	frame = (unsigned short*)malloc(size);

	fwrite( frame, sizeof(unsigned short), 2*size/sizeof(unsigned short), data );

	syslog(LOG_INFO, "%s created\n", file_path_obj.full_path);

	fclose( data );
	if( header ) free( header );
	if( frame ) free( frame );

	return file_path_obj.full_path;
}



extern char * preview(long exp_time_ms, int sock){
	int n;

	short cam_handle;
	short num_frames;

	EXPTIMEMS = exp_time_ms;
	WAITTIMEMS  = 60000;
	TEMPERATURE = -60.0;

	struct config config_obj = read_config();
	struct file_path file_path_obj = file_path_string();

	struct stat st = {0};
	if (stat(file_path_obj.folder_path, &st) == -1) mkdir(file_path_obj.folder_path, 0700);

	FILE *data;
	data = fopen( file_path_obj.full_path , "w" );
	if( !data ) {
		syslog(LOG_INFO, "%s not opened error\n", file_path_obj.full_path);
		return 0;
	}

	struct header image_header = build_header();

	// syslog(LOG_INFO, "exp_time %f\n", image_header.exp_time);
	// syslog(LOG_INFO, "day %d\n", image_header.day);
	// syslog(LOG_INFO, "month %s\n", image_header.month);
	// syslog(LOG_INFO, "year %d\n", image_header.year);
	// syslog(LOG_INFO, "filename %s\n", image_header.filename);
	// syslog(LOG_INFO, "siteName %s\n", image_header.siteName);
	// syslog(LOG_INFO, "filter %s\n", image_header.filter);
	// syslog(LOG_INFO, "LOCATION %s\n", LOCATION);

	unsigned char * header;
	const int header_size = 128;
	header = (unsigned char*)malloc(header_size);
	copy_header(image_header,header);
	fwrite( header, 1, header_size, data);

	n = write(sock,header,header_size);
	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}

	sleep(ceil(image_header.exp_time));

	const unsigned long size = 1024*1024;
	unsigned short * frame;
	frame = (unsigned short*)malloc(size);

	fwrite( frame, sizeof(unsigned short), 2*size/sizeof(unsigned short), data );

	syslog(LOG_INFO, "%s created\n", file_path_obj.full_path);

	n = write(sock,frame,2*size/sizeof(unsigned short));
	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}

	fclose( data );
	if( header ) free( header );
	if( frame ) free( frame );

	return file_path_obj.full_path;
}
