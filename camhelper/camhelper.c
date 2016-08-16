#include <camhelper.h>
#include <syslog.h>
#include <stdbool.h>
#include <stdio.h>

int init_camera(){
	syslog(LOG_ERR, "initialising camera\n");
	if (VERBOSE) printf("initialising camera\n");
	return 1;
}

int uninit_camera(){
	syslog(LOG_ERR, "uninitialising camera\n");
	if (VERBOSE) printf("uninitialising camera\n");
	return 0;
}

int capture(unsigned long exp_time_ms){
	syslog(LOG_INFO, "capture command\n");
	if (VERBOSE) printf("capture command\n");
	return 0;
}

int capture_write(struct Command* command, char* response){
	capture(command->time_us);
	syslog(LOG_INFO, "capture_write command\n");
	if (VERBOSE) printf("capture_write command\n");
	return 0;
}
