#include <getopt.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>

extern int XDIM;
extern int YDIM;
extern int DAY;
extern int YEAR;
extern int XBIN;
extern int YBIN;
extern int NUMSUBARRAYS;
extern int LEFT;
extern int RIGHT;
extern int TOP1;
extern int BOTTOM1;
extern int NUMCOADDS;
extern long EXPTIMEMS; //milliseconds
extern long WAITTIMEMS; //milliseconds
extern float TEMPERATURE;
extern char * SITENAME;
extern char * FILENAME;
extern char * MONTH;
extern char * FILTER;

extern char * LOCATION;

struct header{
	unsigned short xdim, ydim, day, year, xbin, ybin, num_subarrays, left, right, bottom1, top1, num_coadds;
	float exp_time, wait_time; //seconds
	float temperature;
	char *site_name, *filename, *month, *filter;
	double acceleration[3], field[3];
};

struct file_path{
	char *folder_path, *filename, *full_path;
};

struct config{
	char *config_site, *config_cam, *config_path;
};
