#include <phidget21.h>
#include "common.h"

struct compass_data get_compass_data(){
	int result;
	const char *err;
	struct compass_data compass_data_object;
	double acceleration;
	double magnetic_field;
	int i;

	//Declare a spatial handle
	CPhidgetSpatialHandle spatial = 0;

	//create the spatial object
	CPhidgetSpatial_create(&spatial);

	//open the spatial object for device connections
	CPhidget_open((CPhidgetHandle)spatial, -1);

	//get the program to wait for a spatial device to be attached
	if((result = CPhidget_waitForAttachment((CPhidgetHandle)spatial, 10000)))
	{
		CPhidget_getErrorDescription(result, &err);
		printf("Problem waiting for attachment: %s\n", err);
		return compass_data_object;
	}

	int serial_no, version;
	CPhidget_getSerialNumber((CPhidgetHandle)spatial, &serial_no);
	CPhidget_getDeviceVersion((CPhidgetHandle)spatial, &version);
	compass_data_object.serial_no = serial_no;
	compass_data_object.version = version;

	int readerror = 0;
	do {
		for (i = 0; i<3; i++) {
			CPhidgetSpatial_getAcceleration(spatial, i, &acceleration);
			CPhidgetSpatial_getMagneticField(spatial, i, &magnetic_field);
			if (acceleration < 10.0){
				readerror = 0;
				compass_data_object.acceleration[i] = acceleration;
			} else {
				readerror = 1;
			}
			if (magnetic_field < 10.0){
				readerror = 0;
				compass_data_object.magnetic_field[i] = magnetic_field;
			} else {
				readerror = 1;
			}
		}
	} while( readerror );

	CPhidget_close((CPhidgetHandle)spatial);
	CPhidget_delete((CPhidgetHandle)spatial);

	return compass_data_object;
}
