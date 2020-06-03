
#ifndef CUBESAT_DEVICE_I2C_DEVICE
#define CUBESAT_DEVICE_I2C_DEVICE


// See https://github.com/mike-brookes/BeagleBone-I2C/blob/master/Lib/I2C/I2CDevice.h

#include "device/device.h"
#include "exceptions/i2c_exception.h"

#include <sys/stat.h>
#include <unistd.h>


#define I2C_MAX_BUFF 1024

#define I2C_BUS_COUNT 2 // The number of I2C buses available
#define I2C_1 "/dev/i2c-1" // File Path for the i2c-1 bus.
#define I2C_2 "/dev/i2c-2" // File Path for the i2c-2 bus.


namespace cubesat {
	
	/**
	 * @brief Utility class for accessing I2C devices.
	 */
	class I2CDevice : public IDevice {
	public:
		
		/**
		 * @brief Default constructor
		 */
		I2CDevice();
		~I2CDevice();
		
		
		/** 
		 * @brief Initializes the device. Requires the device address and bus ID to be set.
		 */
		void InitDevice() throw(I2CSetupException&);
		
		
		/**
		 * @brief Retrieves a value from a register on the device.
		 * @param register_address The address of the register (e.g. 0x4c).
		 * @return The value read.
		 */
		short GetValueFromRegister(unsigned char register_address);
		
		/**
		 * @brief Reads values from a range of registers on the device.
		 * @param values_out Values read are returned here.
		 * @param start_register_address The first register to read from.
		 * @param end_register_address The last register to read from (inclusive).
		 * @return The number of values read
		 */
		int GetValuesFromRegisters(uint8_t *values_out, unsigned char start_register_address, unsigned char end_register_address);
		
		
		/**
		 * @brief Sets the value that will next be written to the device.
		 * @param register_value The value to write.
		 */
		void SetRegisterValue(unsigned char register_value);
		
		
		/**
		 * @brief Sets the register address that the previously set register value will be written to.
		 * @param register_address
		 */
		void SetRegisterAddress(unsigned char register_address);
		
		
		/**
		 * @brief Performs a write request.
		 * @param buffer_size The buffer size.
		 * @return 
		 */
		int WriteToDevice(size_t buffer_size) throw(I2CSetupException&);
		
		
	protected:
		
		/**
		 * @brief Returns the file handle for I/O with the device.
		 * @return The file handle.
		 */
		int GetDeviceFileHandle();
		
		/**
		 * @brief Returns the file path for I/O with the device (e.g. "/dev/i2c-2").
		 * @return The file path.
		 */
		const char* GetFilePath();
		
		/**
		 * @brief Sets path to all the available buses. As set with I2CBus (struct) and Defines.
		 */
		void SetBusPaths();
		
		/**
		 * @brief Makes sure the bus ID used is valid.
		 * @return 
		 */
		int ValidateBusId() throw(I2CSetupException&);
		
		
		/**
		 * @brief Makes sure the bus path given exists.
		 * @param bus_path
		 * @return 
		 */
		char* ValidateBusPath(char * bus_path) throw( I2CSetupException& );
		
		/**
		 * @brief Selets a bus path for accessing the I2C device.
		 */
		void SelectABusPath();
		
		/**
		 * @brief Stores the device address.
		 * @param _DeviceAddress The device address (e.g. 0x4c).
		 */
		virtual void SetDeviceAddress(unsigned char _DeviceAddress) = 0;
		
		/**
		 * @brief Stores the bus ID
		 * @param bus_id The bus ID
		 */
		virtual void SetBusID(int bus_id) = 0;
		
		
		/**
		 * @brief Attempts to set up an IOCTL connection to the device as a slave.
		 * @return 
		 */
		int ConnectToDevice();
		
		/**
		 * @brief Attempts to open the file handle.
		 * @return 
		 */
		int OpenDevice() throw( I2CSetupException& );
		
		/**
		 * @brief Reads the current buffer from the I2C device.
		 * @param _BufferSize
		 * @return 
		 */
		short ReadDevice(size_t _BufferSize) throw(I2CSetupException&);
		
		
		const char * device_bus_path;
		
		unsigned char device_address;
		unsigned char register_value;
		unsigned char register_address;
		
		char io_buffer[2];
		char write_buffer[1];
		
		char err_message[I2C_MAX_BUFF];
		int file_handle;
		int bus_id;
		
		bool device_initialized;
		
		
		
	private:
		
		/**
		 * @brief Used to store bus paths.
		 */
		struct I2CBus { const char * BusPath; } bus[I2C_BUS_COUNT + 1];
		
		/**
		 * @brief Used to check file paths.
		 */
		struct stat buffer;
		
	};
	
	}


#endif
