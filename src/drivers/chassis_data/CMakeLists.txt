px4_add_module(
	MODULE drivers__chassis_data
	MAIN chassis_data
	COMPILE_FLAGS
	SRCS
		ChassisData.cpp
		ChassisData.hpp
		decodeData.cpp
	DEPENDS
		px4_work_queue
		udp
		can_eth
		uart
		candevice
	)
