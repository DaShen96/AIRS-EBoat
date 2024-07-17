#pragma once
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <nuttx/can/can.h>
#include <nuttx/can.h>
#include <netpacket/can.h>

class CANSocket
{
public:
	CANSocket (/* args */) = default;
	CANSocket (const char* dev);
	~CANSocket ();

	bool setting(unsigned int baudrate);
	void closeDevice(){close(_m_fd);}
	bool openDevice(const char *dev);
	int readData(unsigned int msec);
	int writeData(uint32_t canid, uint8_t datalen, unsigned char *data, bool frame_format);
	unsigned int get_canid(){return _frame.can_id & 0x7FFFFFFF;}
	unsigned char *get_can_recvdata(){return _frame.data;}
	unsigned char get_canlen(){return _frame.can_dlc;}
	//0 -> stand, 1 -> extend
	bool get_frame_format(){return _frame_format;}
private:
	/* data */
	int isopen(){return _m_fd;}
	bool waitForReadyRead(unsigned int msec=1000);

	struct can_frame _frame;
	unsigned int _baudrate;
	int _m_fd{-1};
	//0 -> stand, 1 -> extend
	bool _frame_format;


};
