#include "chassis_data.h"


/*
	CRC16-MODBUS check
	param: 传入需要校验的数据数组
*/
uint16_t ChassisData::crc_check(unsigned char *pendBuffer)
{
//	bool isequation = false;
	uint16_t crc16 = 0xffff;
	uint16_t poly = 0xa001;
	for(int i = 0;i < 6;i++) {
		unsigned char temp = pendBuffer[i];
		crc16 = temp ^ crc16;
		for(int j = 0 ; j < 8 ; j++) {
			if((0x1&crc16) == 1) {
				crc16 = crc16 >> 1;
				crc16 = poly ^ crc16;
			}
			else
				crc16 = crc16 >> 1;
		}
	}
	return crc16;

}

/*
	解析SMC180型号油门传感器数据
*/
void ChassisData::decode_throttle_smc180(unsigned char *data)
{
	_chassisData.timestamp = hrt_absolute_time();
	switch (data[1]) {
		case 0x00:	//P挡
			_chassisData.gear_right = 0;
			break;
		case 0x08:	//R挡
			_chassisData.gear_right = 2;
			break;
		case 0x01:	//D挡
			_chassisData.gear_right = 3;
			break;
		default:
			break;
	}

	switch (data[2]) {
		case 0x00:	//P挡
			_chassisData.gear_left = 0;
			break;
		case 0x08:	//R挡
			_chassisData.gear_left = 2;
			break;
		case 0x01:	//D挡
			_chassisData.gear_left = 3;
			break;
		default:
			break;
	}
	_chassisData.throttle_right = data[3];
	_chassisData.throttle_left = data[4];
	_chassisData.throttle_button = data[5];
	_chassisData.throttle = _chassisData.throttle_left / 100.0f;
	_orb_chassisData_pub.publish(_chassisData);
}

/*
	解析SMC196型号方向盘数据
*/
void ChassisData::decode_throttle_smc196(unsigned char *data)
{
	_chassisData.timestamp = hrt_absolute_time();
	SteeringWheel sw;
	if(data[7] > 0x64) {
		sw.angle = (float)(data[7] - 0xff);
	}
	else sw.angle = data[7];
	sw.number_turns = data[1];
	sw.dir_ = data[0] & 0x80;
	sw.error_sensor1 = data[0]&0x20;
	sw.error_sensor2 = data[0]&0x40;
	_chassisData.steeringwheel = sw.angle;
	_chassisData.have_steeringwheel = true;
	_orb_chassisData_pub.publish(_chassisData);
}
