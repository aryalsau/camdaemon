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
#include <fitsio.h>

struct file_path{
	char *folder_name, *filename;
};

struct config{
	char *site, *camera, *path;
};

struct data{
	unsigned short xdim, ydim, xbin, ybin, num_subarrays, left, right, bottom1, top1, num_coadds;
	long exp_time_ms;
	float temp_c;
	struct tm *time_info;
	struct config config_object;
	short **image;
	double acceleration[3], field[3];
};
