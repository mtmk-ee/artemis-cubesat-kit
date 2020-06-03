
#ifndef CUBESAT_DEVICE_MPU9250
#define CUBESAT_DEVICE_MPU9250

#include "device/i2c_device.h"

namespace cubesat {
	
	class MPU9250 : public I2CDevice {
		static const uint8_t ACCEL_XOUT_H = 0x3b;
		static const uint8_t ACCEL_XOUT_L = 0x3c;
		static const uint8_t ACCEL_YOUT_H = 0x3d;
		static const uint8_t ACCEL_YOUT_L = 0x3e;
		static const uint8_t ACCEL_ZOUT_H = 0x3f;
		static const uint8_t ACCEL_ZOUT_L = 0x40;
		
		
	public:
		MPU9250( unsigned char _DeviceAddress, int _BusId ) {
			this->SetDeviceAddress( _DeviceAddress );
			this->SetBusID( _BusId );
			this->InitDevice( );
		}
		
		
		uint16_t GetAccelX() {
			return GetValueFromRegister(0x3b);
		}
		
		
		
	protected:
		void SetDeviceAddress( unsigned char _DeviceAddress ){ this->device_address = _DeviceAddress; }
		void SetBusID( int _BusId ) { this->bus_id = _BusId; }
	};
	
}


#endif
