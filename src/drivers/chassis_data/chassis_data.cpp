
#include "chassis_data.h"
#include <px4_platform_common/time.h>
#include <unistd.h>
#include <stdio.h>

px4::AppState ChassisData::appState;

ChassisData::ChassisData()
{
	_chassisData.have_steeringwheel = false;
	_chassisData.steeringwheel = 0.0f;
	_chassisData.throttle = 0.0f;
}

ChassisData::~ChassisData()
{
	#ifdef UDP_CHASSIS
		delete _udp;
	#endif

	#ifdef UART_CHASSIS
		delete _uart;
	#endif

	#ifdef CAN_CHASSIS
		delete _iocan;
	#endif
}

bool ChassisData::init()
{
	#ifdef UDP_CHASSIS
		_udp = new UdpSocket(udpServer_port);
		if(_udp == NULL) {
			printf("failed to open udp\n");
			return false;
		}
	#endif

	#ifdef UART_CHASSIS
		_uart = new Uart(_uartPortName,_uartBaudRate);
		if(_uart == NULL) {
			printf("failed to open uart\n");
			return false;
		}
	#endif

	#ifdef CAN_CHASSIS
		_iocan = new CANSocket("can0");
		if(_iocan == NULL) {
			printf("failed to open can\n");
			return false;
		}
	#endif

	return true;
}

int ChassisData::run()
{
	appState.setRunning(true);

	if ( !init()){
		printf("Chassis Data Failed To Init !!!\n");
		return 0;
	}

	while (!appState.exitRequested()) {
		#ifdef CAN_CHASSIS
			if(_iocan->readData(1000)>0) {
				_canID = _iocan->get_canid();
				memcpy(_canBuffer, _iocan->get_can_recvdata(), 8);
				//_iocan->writeData(_canID, 8, _canBuffer, _iocan->get_frame_format());
				switch (_canID) {
					case SMC180_ID:
						decode_throttle_smc180(_canBuffer);
						break;
					case SMC196_ID:
						decode_throttle_smc196(_canBuffer);
						break;
					default:
						break;
				}
			}
		#endif

		#ifdef UART_CHASSIS
			_uart->send(_sendData,sizeof(_sendData));
			if(_uart->receive(_uartBuffer,sizeof(_uartBuffer))) {
				switch (_uartBuffer[0])	 { //检查地址码
					case STEERINGWHEEL:
						for(int i = 0;i<4;i++) {
							_pendProcessData[i] = _uartBuffer[i+2];
						}
						_chassisData.timestamp = (int)time((time_t*) NULL);
						_chassisData.steeringwheel = decode_steerwheel(_pendProcessData);
						_chassisData.throttle = 0.2f;
						_orb_chassisData_pub.publish(_chassisData);
						break;
					case THROTTLE:
						for(int i = 0;i<4;i++) {
							_pendProcessData[i] = _uartBuffer[i+2];
						}
						_chassisData.timestamp = (int)time((time_t*) NULL);
						//chassisData.throttle = decodeThrottle(_pendProcessData);
						_orb_chassisData_pub.publish(_chassisData);
						break;
					default:
						break;
				}
			}
		#endif

		#ifdef UDP_CHASSIS
			if(udp->receive(_recvCANbuffer,sizeof(_recvCANbuffer))) {
				_eth_can.CanToEth(_recvCANbuffer);
				_canID = eth_can.getRecvID();
				// eth_can.EthToCan(eth_can.getRecvData(),eth_can.getRecvID(),8);
				// udp->send(eth_can.getSendData(),13,"192.168.0.101",4501);
				switch (_canID) {
					case SMC180_ID:
						decode_throttle_smc180(_eth_can.getRecvData());
						break;
					case SMC196_ID:
						decode_throttle_smc196(_eth_can.getRecvData());
						break;
					default:
						break;
				}
			}
		#endif

	}

	return 0;
}
