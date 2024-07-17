
#include "chassis_data.h"

#include <px4_platform_common/log.h>
#include <px4_platform_common/app.h>
#include <px4_platform_common/tasks.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>

static int daemon_task;             /* Handle of deamon task / thread */

//using namespace px4;

extern "C" __EXPORT int chassis_data_main(int argc, char *argv[]);
int chassis_data_main(int argc, char *argv[])
{

	if (argc < 2) {
		PX4_WARN("usage: chassis_data {start|stop|status}\n");
		return 1;
	}

	if (!strcmp(argv[1], "start")) {

		if (ChassisData::appState.isRunning()) {
			PX4_INFO("already running\n");
			/* this is not an error */
			return 0;
		}

		daemon_task = px4_task_spawn_cmd("chassis_data",
						 SCHED_DEFAULT,
						 SCHED_PRIORITY_MAX - 5,
						 2000,
						 PX4_MAIN,
						 (argv) ? (char *const *)&argv[2] : (char *const *)nullptr);

		return 0;
	}

	if (!strcmp(argv[1], "stop")) {
		ChassisData::appState.requestExit();
		return 0;
	}

	if (!strcmp(argv[1], "status")) {
		if (ChassisData::appState.isRunning()) {
			PX4_INFO("is running\n");

		} else {
			PX4_INFO("not started\n");
		}

		return 0;
	}

	PX4_WARN("usage: chassis_data_main {start|stop|status}\n");
	return 1;
}
