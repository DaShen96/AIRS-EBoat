
#include "chassis_data.h"

#include <px4_platform_common/app.h>
#include <px4_platform_common/init.h>
#include <stdio.h>

int PX4_MAIN(int argc, char **argv)
{
	px4::init(argc, argv, "chassis_data");

	printf("chassis_data\n");
	ChassisData chassisdata;
	chassisdata.run();

	printf("goodbye\n");
	return 0;
}
