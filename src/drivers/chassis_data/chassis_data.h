
#pragma once

#include <px4_platform_common/app.h>

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

#define CAN_CHASSIS

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

class ChassisData
{
public:
	ChassisData();
	~ChassisData();

	int run();

	static px4::AppState appState; /* track requests to terminate app */

private:

	chassis_data_s	 _chassisData;

#ifdef UART_CHASSIS
	char *_uartPortName=(char *)"/dev/ttyS4";
	int _uartBaudRate = 115200;
	int _minAngle = -60;
	int _maxAngle = 60;
	int _maxThrottle = 180;
	#define STEERINGWHEEL 0x40
	#define THROTTLE 0x41
	unsigned char _pendProcessData[4];
	unsigned char _uartBuffer[8];
	unsigned char _sendData[8] = {0x31 , 0x32 , 0x33 , 0x34 , 0x35 , 0x36 , 0x37 , 0x38};
	Uart *_uart;
#endif

#ifdef UDP_CHASSIS
	#define udpServer_port 4500		//本地端口
	unsigned char _recvCANbuffer[13];
	UdpSocket  _udp;
	CanEth _eth_can;
#endif

#ifdef CAN_CHASSIS
	unsigned char _canBuffer[8];
	unsigned long _canID;
	CANSocket *_iocan;
#endif
	bool init();
	float decode_steerwheel(unsigned char *data);
	void decode_throttle_smc180(unsigned char *data);
	void decode_throttle_smc196(unsigned char *data);
	uint16_t crc_check(unsigned char *pendBuffer);

	// Publications
	uORB::Publication<chassis_data_s> _orb_chassisData_pub{ORB_ID(chassis_data)};

};
