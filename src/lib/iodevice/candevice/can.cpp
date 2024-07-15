#include "can.hpp"

CANSocket ::CANSocket (const char* dev)
{
    if(0<isopen())
    {
        int flag;
        struct sockaddr_can addr_can;
        struct ifreq ifr;

        if((_m_fd = socket(PF_CAN,SOCK_RAW,CAN_RAW)) < 0)
        {
            perror("Failed to open socket");
            return;
        }

        strcpy(ifr.ifr_name,dev);
        if(-1 == ioctl(_m_fd,SIOCGIFINDEX, &ifr))
        {
            perror("Failed to open socket");
            closeDevice();
            return;
        }

        /* Disable loopback */
        flag = 1;
        if (-1 == setsockopt(_m_fd, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &flag, sizeof(flag)))
        {
            perror("Failed to set setsockopt");
            closeDevice();
            return;
        }
        /* Disable receiving own message */
        flag = 1;
        if (-1 == setsockopt(_m_fd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &flag, sizeof(flag)))
        {
            perror("Failed to set setsockopt");
            closeDevice();
            return;
        }
        /* Use AF_CAN protocol family */
        addr_can.can_family = AF_CAN;
        addr_can.can_ifindex = ifr.ifr_ifindex;
        /* Binding socket */
        if (-1 == bind(_m_fd, (struct sockaddr*)&addr_can, sizeof(addr_can)))
        {
            perror("Failed to bind can socket");
            closeDevice();
            return;
        }

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
        int flag;
	struct sockaddr_can addr_can;
	struct ifreq ifr;

        if((_m_fd = socket(PF_CAN,SOCK_RAW,CAN_RAW)) < 0)
        {
            perror("Failed to open socket");
            return false;
        }

        strcpy(ifr.ifr_name,dev);
        if(-1 == ioctl(_m_fd,SIOCGIFINDEX, &ifr))
        {
            closeDevice();
            return false;
        }

        /* Disable loopback */
        flag = 1;
        if (-1 == setsockopt(_m_fd, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &flag, sizeof(flag))) {
            closeDevice();
            return false;
        }
        /* Disable receiving own message */
        flag = 1;
        if (-1 == setsockopt(_m_fd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &flag, sizeof(flag))) {
            closeDevice();
            return false;
        }
        /* Use AF_CAN protocol family */
        addr_can.can_family = AF_CAN;
        addr_can.can_ifindex = ifr.ifr_ifindex;
        /* Binding socket */
        if (-1 == bind(_m_fd, (struct sockaddr*)&addr_can, sizeof(addr_can))) {
            closeDevice();
            return false;
        }

    }

    return (isopen());

}

/**
 * @brief 设置CAN端口波特率，在终端使用命令“ sudo ip link set can0 up type can bitrate 500000 ”打开设备
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
		return nbytes;
	}
	return -1;
}

/**
 * @brief 发送CAN数据
 * @return 成功：字节数 ， 失败： -1
 */
int CANSocket::writeData(uint32_t canid, uint8_t datalen, unsigned int *data)
{
    _frame.can_id = canid;
    _frame.can_dlc = datalen;
    memcpy(_frame.data, data, 8);
   int nbytes = write(_m_fd,&_frame,sizeof(_frame));
    if(nbytes < 0)
    {
	printf("can read error\n");
	return -1;
    }
    return nbytes;
}
