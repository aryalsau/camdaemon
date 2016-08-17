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
	char* location;
	char* file_name;
	char* folder_name;
	char* folder_path;
	char* file_path;
	struct tm *time_info;
	unsigned long exp_time_us;
	float temp_c;
	float grav_field[3];
	float mag_field[3];
	unsigned char xbin;
	unsigned char ybin;
	unsigned short xdim;
	unsigned short ydim;
	unsigned short** imagedata;
};

int update_config(char* filepath, struct Data* data);
int update_exp_time(unsigned long* exp_time_us, struct Data* data);
int update_xbin(unsigned char* xbin, struct Data* data);
int update_ybin(unsigned char* ybin, struct Data* data);
int update_file_name(struct Data* data);
int write_to_disk(struct Data* data, char* response);
int allocate_frame(struct Data* data);
int free_frame(struct Data* data);

#endif
