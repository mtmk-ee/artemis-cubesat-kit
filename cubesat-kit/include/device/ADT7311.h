
#ifndef CUBESAT_DEVICE_ADT7311
#define CUBESAT_DEVICE_ADT7311

#include "device/i2c_device.h"
#include "device/spi/SPIDevice.h"

namespace cubesat {
	
	
	// https://www.analog.com/media/en/technical-documentation/data-sheets/ADT7311.pdf
	class ADT7311 : public SPIDevice {
	public:
		ADT7311() : SPIDevice(0, 0) {}
		ADT7311(unsigned int bus, unsigned int device) : SPIDevice(bus, device) {}
		virtual ~ADT7311() {}
		
		
		float GetTemperature() {
			// TODO
			return 0;		
		}
	};
	
}


#endif
