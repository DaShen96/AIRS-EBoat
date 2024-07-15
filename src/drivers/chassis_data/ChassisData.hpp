#pragma once

#include <px4_platform_common/log.h>
#include <px4_platform_common/defines.h>
#include <px4_platform_common/module.h>
#include <px4_platform_common/module_params.h>
#include <px4_platform_common/posix.h>
#include <px4_platform_common/px4_work_queue/ScheduledWorkItem.hpp>

#include <drivers/drv_hrt.h>
#include <lib/perf/perf_counter.h>
#include <lib/iodevice/udp/udp.h>
#include <lib/iodevice/uart/uart.h>
#include <lib/iodevice/can_eth/can_eth.h>
#include "lib/iodevice/candevice/can.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <termios.h>

#include <uORB/Publication.hpp>
#include <uORB/Subscription.hpp>
#include <uORB/SubscriptionInterval.hpp>
#include <uORB/topics/parameter_update.h>
#include <uORB/topics/chassis_data.h>

#define CAN

#define SMC180_ID 0x00F8FFFF	//型号SMC180油门传感器的默认CANID
#define SMC196_ID 0x0CFDD901	//型号SMC180油门传感器的默认CANID

using namespace time_literals;

typedef struct
{
	bool dir_;	//true 顺时针 ， false 逆时针
	bool error_sensor1;	//传感器状态
	bool error_sensor2;	//传感器状态
	float angle;	// 小于0%的用0xff减去读到的数值
	unsigned char number_turns;	//转动的圈数
	float sync_angle;	//舵角同步
}SteeringWheel;

class ChassisData : public ModuleBase<ChassisData>, public ModuleParams, public px4::ScheduledWorkItem
{

public:
	ChassisData();
	~ChassisData() override;

	/** @see ModuleBase */
	static int task_spawn(int argc, char *argv[]);

	/** @see ModuleBase */
	static int custom_command(int argc, char *argv[]);

	/** @see ModuleBase */
	static int print_usage(const char *reason = nullptr);

	bool init();

	int print_status() override;

	void parameters_update(bool force);

	float decode_steerwheel(unsigned char *data);
	void decode_throttle_smc180(unsigned char *data);
	void decode_throttle_smc196(unsigned char *data);
	uint16_t crc_check(unsigned char *pendBuffer);


private:
	void Run() override;

	char *_uartPortName=(char *)"/dev/ttyS4";
	int _uartBaudRate = 115200;
	int _minAngle = -60;
	int _maxAngle = 60;
	int _maxThrottle = 180;

	bool _isInit{false};
	chassis_data_s	 _chassisData;

#ifdef UART
	#define STEERINGWHEEL 0x40
	#define THROTTLE 0x41
	unsigned char _pendProcessData[4];
	unsigned char _uartBuffer[8];
	unsigned char _sendData[8] = {0x31 , 0x32 , 0x33 , 0x34 , 0x35 , 0x36 , 0x37 , 0x38};
	Uart *_uart;
#endif

#ifdef UDP
	#define udpServer_port 4500		//本地端口
	unsigned char _recvCANbuffer[13];
	UdpSocket  _udp;
	CanEth _eth_can;
#endif

#ifdef CAN
	uint8_t _canBuffer[8];
	uint32_t _canID;
	CANSocket *_iocan;
#endif

	// Publications
	uORB::Publication<chassis_data_s> _orb_chassisData_pub{ORB_ID(chassis_data)};

	// Performance (perf) counters
	perf_counter_t	_loop_perf{perf_alloc(PC_ELAPSED, MODULE_NAME": cycle")};
	perf_counter_t	_loop_interval_perf{perf_alloc(PC_INTERVAL, MODULE_NAME": interval")};

	DEFINE_PARAMETERS(
		(ParamInt<px4::params::UART_PORTNAME>) _param_uart_portname,
		(ParamInt<px4::params::UART_BAUDRATE>) _param_uart_baudrate,
		(ParamInt<px4::params::MIN_ANGLE>) _param_min_angle,
		(ParamInt<px4::params::MAX_ANGLE>) _param_max_angle,
		(ParamInt<px4::params::MAX_THROTTLE>) _param_max_throttle
	)

	// Subscriptions
	uORB::SubscriptionInterval _parameter_update_sub{ORB_ID(parameter_update), 1_s};
};
