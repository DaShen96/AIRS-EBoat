/*获取底盘数据，如方向盘、油门等，原始数据经过解析后，通过uORB发布*/


#include "ChassisData.hpp"

ChassisData::ChassisData() :
	ModuleParams(nullptr),
	ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::lp_default)
{
	_chassisData.have_steeringwheel = false;
	_chassisData.steeringwheel = 0.0f;
	_chassisData.throttle = 0.0f;
}

ChassisData::~ChassisData()
{
	_isInit = false;
	#ifdef UDP
		delete _udp;
	#endif

	#ifdef UART
		delete _uart;
	#endif

	#ifdef CAN
		delete _iocan;
	#endif

	perf_free(_loop_perf);
	perf_free(_loop_interval_perf);
}

void ChassisData::parameters_update(bool force)
{
	// check for parameter updates
	if (_parameter_update_sub.updated() || force) {
		// // clear update
		parameter_update_s pupdate;
		_parameter_update_sub.copy(&pupdate);

		// update parameters from storage
		updateParams();

		switch (_param_uart_portname.get()) {
			case 0:
				_uartPortName = (char *)"/dev/ttyS0";
				break;
			case 1:
				_uartPortName = (char *)"/dev/ttyS1";
				break;
			case 2:
				_uartPortName = (char *)"/dev/ttyS3";
				break;
			case 3:
				_uartPortName = (char *)"/dev/ttyS4";
				break;
			case 4:
				_uartPortName = (char *)"/dev/ttyS6";
				break;
			case 5:
				_uartPortName = (char *)"/dev/ttyS7";
				break;
			default:
				break;
		}

		_uartBaudRate = _param_uart_baudrate.get();

	}
}

bool ChassisData::init()
{
	// parameters_update(true);
	ScheduleOnInterval(20000_us);//单位是微秒
	return true;
}

void ChassisData::Run()
{
	if (should_exit()) {
		ScheduleClear();
		exit_and_cleanup();
		return;
	}

	parameters_update(true);

	if(_isInit==false) {
		_isInit = true;

	#ifdef UDP
		_udp = new UdpSocket(udpServer_port);
	#endif

	#ifdef UART
		_uart = new Uart(_uartPortName,_uartBaudRate);
	#endif

	#ifdef CAN
		_iocan = new CANSocket("can0");
		if(_iocan == NULL) {
			printf("failed to open can\n");
			return;
		}
	#endif
	}

#ifdef CAN
	if(_iocan->readData(1000)) {
		_canID = _iocan->get_canid();
		memcpy(_canBuffer,_iocan->get_can_recvdata(),8);
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

#ifdef UART
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

#ifdef UDP
	if(udp->receive(_recvCANbuffer,sizeof(_recvCANbuffer))) {
		eth_can.CanToEth(_recvCANbuffer);
		_canID = eth_can.getRecvID();
		// eth_can.EthToCan(eth_can.getRecvData(),eth_can.getRecvID(),8);
		// udp->send(eth_can.getSendData(),13,"192.168.0.101",4501);
		switch (_canID) {
			case SMC180_ID:
				decode_throttle_smc180(eth_can.getRecvData());
				break;
			case SMC196_ID:
				decode_throttle_smc196(eth_can.getRecvData());
				break;
			default:
				break;
		}
	}
#endif

	// perf_end(_loop_perf);
}

int ChassisData::task_spawn(int argc, char *argv[])
{
	ChassisData *instance = new ChassisData();

	if (instance) {
		_object.store(instance);
		_task_id = task_id_is_work_queue;

		if (instance->init()) {
			return PX4_OK;
		}

	} else {
		PX4_ERR("alloc failed");
	}

	delete instance;
	_object.store(nullptr);
	_task_id = -1;

	return PX4_ERROR;
}

int ChassisData::print_status()
{
	PX4_INFO("running... \n");

	return 0;
}

int ChassisData::custom_command(int argc, char *argv[])
{
	return print_usage("unknown command");
}

int ChassisData::print_usage(const char *reason)
{
	if (reason) {
		PX4_WARN("%s\n", reason);
	}

	PRINT_MODULE_DESCRIPTION(
		R"DESCR_STR(
### Description
	recive steering wheel and throttle raw data,then publish them with uORB;

)DESCR_STR");

	PRINT_MODULE_USAGE_NAME("chassis_data", "template");
	PRINT_MODULE_USAGE_COMMAND("start");
	PRINT_MODULE_USAGE_DEFAULT_COMMANDS();

	return 0;
}


extern "C" __EXPORT int chassis_data_main(int argc, char *argv[])
{
	return ChassisData::main(argc, argv);
}
