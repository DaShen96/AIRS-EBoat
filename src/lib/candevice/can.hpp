#pragma once
#include <nuttx/config.h>
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

class IOCan
{
public:
	IOCan (/* args */);
	~IOCan ();

	struct can_frame frame;

	bool waitForReadyRead(unsigned int msec=1000);
	bool setting(unsigned int baudrate);
	int isopen(){return _m_fd;}
	void closeDevice(){close(_m_fd);}
	bool openDevice(const char* dev);
	int readData(unsigned int msec);
	int writeData();
private:
	/* data */
	unsigned int _baudrate;
	int _m_fd{-1};

};
