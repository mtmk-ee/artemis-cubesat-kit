
#ifndef CUBESAT_DEVICE_OPT3001
#define CUBESAT_DEVICE_OPT3001

#include "device/i2c_device.h"
#include "device/spi/SPIDevice.h"

namespace cubesat {
	
	
	class OPT3001 : public I2CDevice {
	public:
		OPT3001() : I2CDevice() {}
		OPT3001(unsigned int bus, unsigned int device) : I2CDevice() {}
		virtual ~OPT3001() {}
		
		
		float GetTemperature() {
			// TODO
			return 0;		
		}
		
	protected:
		void SetDeviceAddress( unsigned char _DeviceAddress ){ this->device_address = _DeviceAddress; }
		void SetBusID( int _BusId ) { this->bus_id = _BusId; }
	};
	
}


#endif
