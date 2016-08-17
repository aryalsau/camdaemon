#ifndef _COMMON
#define _COMMON

extern bool VERBOSE;

typedef enum {CAPTURE, STOP, INVALID} Flag;

struct Command {
	Flag flag;
	unsigned long exp_time_us;
	unsigned char xbin;
	unsigned char ybin;
};

struct Data {
	char* site;
	char* camera;
	char* root;
	char* file_name;
	char* folder_name;
	struct tm *time_info;
	unsigned long exp_time_us;
	float temp_c;
	float grav_field[3];
	float mag_field[3];
	unsigned char xbin;
	unsigned char ybin;
	unsigned short xdim;
	unsigned short ydim;
	short** imagedata;
};

int update_config(char* filepath, struct Data* data);
int update_exptime(unsigned long* exp_time_us, struct Data* data);
int update_xbin(unsigned char* xbin, struct Data* data);
int update_ybin(unsigned char* ybin, struct Data* data);
int update_filename(struct Data* data);
int write_to_disk(struct Data* data, char* response);

#endif
