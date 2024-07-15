/**
 * @file can_eth.h
 *
 *  Converting CAN data to ETH data
 *
 * @author dashen
 */

#ifndef CAN_ETH_H_
#define CAN_ETH_H_

#include <stdio.h>
#include <string.h>

class CanEth{
private:

        uint32_t _recvID;
        unsigned char _dataLen;
        unsigned char _recvData[8]{};
        unsigned char _sendData[13]{};

        //true -> stand  ;  false -> extend
        bool _frameID{false};
public:
        CanEth(){}

        ~CanEth(){}


        //CAN设备数据转换为ETH数据，message len 13 bytes
        void CanToEth(unsigned char *message);
        //ETH数据转换为CAN数据，message len 8 bytes
        void EthToCan(unsigned char *message , uint32_t sendID , u_char datalen);

        uint32_t getRecvID(){return _recvID;}

        u_char *getRecvData(){return _recvData;}

        u_char *getSendData(){return _sendData;}

        char getDataLen(){return _dataLen;}

        bool standOrExtend(){return _frameID;}

};

#endif
