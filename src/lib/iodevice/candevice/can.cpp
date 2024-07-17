#include "can.hpp"

CANSocket ::CANSocket (const char* dev)
{
        struct sockaddr_can addr_can;
        struct ifreq ifr;
        printf("CAN Init!!! \n");
        if((_m_fd = socket(PF_CAN,SOCK_RAW,CAN_RAW)) < 0)
        {
            printf("Failed to open socket\n");
            return;
        }

        strcpy(ifr.ifr_name,dev);
        if(0 < ioctl(_m_fd,SIOCGIFINDEX, &ifr))
        {
            printf("Failed to open socket\n");
            closeDevice();
            return;
        }
        /* Use AF_CAN protocol family */
        addr_can.can_family = AF_CAN;
        addr_can.can_ifindex = ifr.ifr_ifindex;
        /* Binding socket */
        if (-1 == bind(_m_fd, (struct sockaddr*)&addr_can, sizeof(addr_can)))
        {
            printf("Failed to bind can socket\n");
            closeDevice();
            return;
        }

}

CANSocket ::~CANSocket ()
{
    if(isopen())
        closeDevice();
}

/**
 * @brief 等待数据，设置超时时间
 * @param msec 超时时间 ms
 * @return 成功：>0 ， 失败： <=0
 */
bool CANSocket::waitForReadyRead(unsigned int msec)
{
    struct timeval tv;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(_m_fd, &fds);
    tv.tv_sec = msec / 1000;
    tv.tv_usec = (msec * 1000) % 1000000;
    return (select(_m_fd + 1, &fds, NULL, NULL, &tv) > 0); //tv(NULL)-block, tv(sec=0, usec=0)-no timeout
}

/**
 * @brief 打开CAN设备
 * @param dev 要打开的CAN设备号
 * @return 成功：true ， 失败： false
 */
bool CANSocket::openDevice(const char* dev)
{
    if(0<isopen())
    {
        struct sockaddr_can addr_can;
        struct ifreq ifr;
        printf("CAN Init!!! \n");
        if((_m_fd = socket(PF_CAN,SOCK_RAW,CAN_RAW)) < 0)
        {
            printf("Failed to open socket\n");
            return false;
        }

        strcpy(ifr.ifr_name,dev);
        if(0 < ioctl(_m_fd,SIOCGIFINDEX, &ifr))
        {
            printf("Failed to open socket\n");
            closeDevice();
            return false;
        }
        /* Use AF_CAN protocol family */
        addr_can.can_family = AF_CAN;
        addr_can.can_ifindex = ifr.ifr_ifindex;
        /* Binding socket */
        if (-1 == bind(_m_fd, (struct sockaddr*)&addr_can, sizeof(addr_can)))
        {
            printf("Failed to bind can socket\n");
            closeDevice();
            return false;
        }

    }
    return (isopen());

}

/**
 * @brief 设置CAN端口波特率
 * @param baudrate 波特率
 * @return 成功：true ， 失败： false
 */
bool CANSocket::setting(unsigned int baudrate)
{
    return true;
}

/**
 * @brief 接收CAN数据
 * @param msec 超时时间
 * @return 成功：字节数 ， 失败： -1
 */
int CANSocket::readData(unsigned int msec)
{
	if(waitForReadyRead(msec))
	{
		int nbytes = read(_m_fd,&_frame,sizeof(_frame));
		if(nbytes < 0)
		{
			printf("can read error\n");
			return -1;
		}
        _frame_format = (_frame.can_id & CAN_EFF_FLAG) > 0;
		return nbytes;
	}
	return -1;
}

/**
 * @brief 发送CAN数据
 * @param canid 报文ID
 * @param datalen 报文长度
 * @param data 报文数据
 * @param frame_format 报文格式，0 -> stand, 1 -> extend
 * @return 成功：字节数 ， 失败： -1
 */
int CANSocket::writeData(uint32_t canid, uint8_t datalen, unsigned char *data, bool frame_format)
{
    if( canid > 0x7FF || frame_format )
        _frame.can_id = canid | CAN_EFF_FLAG;
    else
        _frame.can_id = canid;

    _frame.can_dlc = datalen;
    memcpy(_frame.data, data, 8);
   int nbytes = write(_m_fd,&_frame,sizeof(_frame));
    if(nbytes < 0)
    {
	printf("can write error\n");
	return -1;
    }
    return nbytes;
}
