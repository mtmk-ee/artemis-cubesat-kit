
#ifndef CUBESAT_DEVICE_GENERIC_DEVICE
#define CUBESAT_DEVICE_GENERIC_DEVICE

#include <string>

namespace cubesat {

	
	class GenericDevice {
	public:
		virtual ~GenericDevice();
		
	protected:
		
	private:
		
	};
	
	
	class I2CDevice : public GenericDevice {
	public:
		
	protected:
		
	};
	
	class SPIDevice : public GenericDevice {
		
	};
	
	class UARTDevice : public GenericDevice {
		
	};
	


	
}


#endif
