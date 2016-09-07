#include <phidget21.h>
#include <syslog.h>
#include <stdbool.h>
#include <stdio.h>
#include "../common.h"
#include "../compass.h"

CPhidgetSpatialHandle spatial = 0; //Declare a spatial handle

int init_compass(void){

	const char *err;
	int result;

	syslog(LOG_ERR, "initialising compass\n");
	if (VERBOSE) printf("initialising compass\n");

	CPhidgetSpatial_create(&spatial); //create the spatial object
	CPhidget_open((CPhidgetHandle)spatial, -1); //open the spatial object for device connections

	if((result = CPhidget_waitForAttachment((CPhidgetHandle)spatial, 100))) {
		CPhidget_getErrorDescription(result, &err);
		printf("Problem waiting for attachment: %s\n", err);
		return 0;
	}

	int serial_no, version;
	CPhidget_getSerialNumber((CPhidgetHandle)spatial, &serial_no);
	CPhidget_getDeviceVersion((CPhidgetHandle)spatial, &version);

	return 1;
}


int uninit_compass(void){
	syslog(LOG_ERR, "uninitialising compass\n");
	if (VERBOSE) printf("uninitialising compass\n");

	CPhidget_close((CPhidgetHandle)spatial);
	CPhidget_delete((CPhidgetHandle)spatial);

	return 0;
}


int acquire_compass_fielddata(struct Data* data){
	syslog(LOG_ERR, "retrieving compass data\n");
	if (VERBOSE) printf("retrieving compass data\n");

	double grav_field;
	double mag_field;

	bool readerror = false;
	unsigned char j = 0;
	do {
		for (unsigned char i = 0; i<3; i++) {
			CPhidgetSpatial_getAcceleration(spatial, i, &grav_field);
			CPhidgetSpatial_getMagneticField(spatial, i, &mag_field);
			if (grav_field < 10.0){
				readerror = false;
				data->grav_field[i] = grav_field;
			} else {
				readerror = true;
			}
			if (mag_field < 10.0){
				readerror = false;
				data->mag_field[i] = mag_field;
			} else {
				readerror = true;
			}
		}
		j++;
	} while( readerror && (j<5) );

	return 0;
}
