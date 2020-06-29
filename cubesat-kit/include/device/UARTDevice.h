
#ifndef CUBESAT_DEVICE_UARTDEVICE
#define CUBESAT_DEVICE_UARTDEVICE

#include <string>
#include <stdint.h>
#include <termios.h>

namespace cubesat {
	
	/**
	 * @class UARTDevice
	 * @brief 
	 */
	class UARTDevice {
	public:
	
		UARTDevice() {}
		UARTDevice(unsigned int device, unsigned int baud_rate);
		virtual ~UARTDevice();
		
		virtual int Open();
		virtual void Close();
		
		
		virtual void WriteByte(uint8_t byte);
		virtual void Write(uint8_t byte[], unsigned int len);
		virtual size_t Read(uint8_t out[], size_t len);
		virtual void Drain();
		
		
		
	private:
		std::string filename;
		unsigned int device, baud_rate;
		int file;
		
		
		struct termios uartTermios, oldDescriptor;
	};

}

#endif
