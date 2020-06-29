
#ifndef CUBESAT_DEVICE_I2CDEVICE
#define CUBESAT_DEVICE_I2CDEVICE

#include <stdint.h>


namespace cubesat {
	
	
	class I2CDevice {
	public:
		I2CDevice();
		I2CDevice(uint8_t bus, uint8_t device);
		virtual ~I2CDevice();
		
		virtual int Open();
		virtual void Close();
		
		
		virtual int Write(uint8_t value);
		virtual int WriteRegister(uint8_t register_addr, uint16_t value);
		virtual int ReadRegisters(uint8_t *out, uint8_t first_addr, uint8_t len);
		virtual uint16_t ReadRegister(uint8_t register_addr);
		
		
	private:
		int bus;
		int device;
		int file;
	};

}

#endif
