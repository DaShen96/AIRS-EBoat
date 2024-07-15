/**
 * @file can_eth.cpp
 *
 */
#include "can_eth.h"

void CanEth::CanToEth(unsigned char *message)
{
	_frameID = (message[0] & 0x80) > 0;
	_dataLen = message[0] & 0x0f;
	_recvID = message[1]<<24 | message[2]<<16 | message[3]<<8 | message[4];
	memset(_recvData , 0 , sizeof(_recvData));
	for(int i = 0 ; i<8 ; i++)
	{
		_recvData[i] = message[i+5];
	}

}

/**
 * ETH数据转换为CAN数据，message len 8 bytes
 * @param 1: 要发送的数据buffer，不超过8个字节
 * @param 2: 该帧数据的CANID
 * @param 3: 数据大小，不超过8个字节
 *
 */
void CanEth::EthToCan(unsigned char *message , uint32_t sendID , u_char datalen)
{
	if (datalen>8)	return;		//CAN一帧最多8个字节数据
	memset(_sendData,0,sizeof(_sendData));
	if(sendID > 0x7ff)	//判断是标准帧还是扩展帧
	{
		_frameID = false;
	}
	else _frameID = true;
	_sendData[0] |=  _frameID ? (datalen) : (datalen | 0x80);
	for(int i = 0 ; i<4 ; i++)
	{
		_sendData[i+1] = (sendID>>8*(3-i)) & 0xff;
	}
	for(int i = 0 ; i<8 ; i++)
	{
		_sendData[i+5] = message[i];
	}
}


