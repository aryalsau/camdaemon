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
    unsigned short xdim, ydim, day, year, xbin, ybin, numSubarrays, left, right, bottom1, top1, numCoadds;
    float expTime, waitTime; //seconds
    float temperature;
    char *siteName, *filename, *month, *filter;
};

struct filepath{
    char *folderpathptr, *filenameptr, *fullpathptr;
};

struct config{
    char *configSite, *configCam, *configPath;
};
