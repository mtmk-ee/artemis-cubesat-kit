
#include "device/i2c_device.h"

#include <cstdlib>
#include <iostream>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <cstring>


using namespace cubesat;


I2CDevice::I2CDevice() {
	this->device_address = 0x00;
	this->bus_id = 0;
	this->device_initialized = false;
}

I2CDevice::~I2CDevice() {
	close(this->file_handle);
}

void I2CDevice::InitDevice() throw( I2CSetupException& ) {
	if(!this->device_address) throw I2CSetupException( "I2C Device Not Configured ( try : 'obj->SetDeviceAddress([hex address])' )" );
	if(!this->bus_id) throw I2CSetupException( "I2C Device Not Configured ( try : 'obj->SetBusId([bus number])' )" );
	/*
	 * ** ## -- Setup Stage -- ## ** *
	 * SetBusPaths : Saves the file paths to the available buses for ease of access.
	 */
	this->SetBusPaths( );

	/*
	 * ** ## -- Assignment Stage ( based on args ) -- ## ** *
	 * ValidateBusId : Make sure we have a valid bus ID before proceeding.
	 * SelectABusPath : Used to specify which bus your I2C device is on.
	 * SetDeviceAddress: Hex value for your specific I2C Device.
	 */
	this->ValidateBusId( );
	this->SelectABusPath( );

	/*
	 * ** ## -- Init Stage -- ## ** *
	 * OpenDevice : Creates a file handle for the device, should it be closed? Probably... :)
	 * ConnectToDevice : Assigns the device as an I2C Slave and checks availability using IOCTL
	 *
	 * More info on IOCTL : http://man7.org/linux/man-pages/man2/ioctl.2.html
	 */
	this->OpenDevice();
	this->ConnectToDevice();

	this->device_initialized = true;
}

void I2CDevice::SetBusPaths( ) {
	this->bus[1].BusPath = this->ValidateBusPath((char *)I2C_1);
	this->bus[2].BusPath = this->ValidateBusPath((char *)I2C_2);
}

void I2CDevice::SelectABusPath( ) {
	this->device_bus_path = bus[ this->bus_id ].BusPath;
}

void I2CDevice::SetRegisterAddress( unsigned char _RegisterAddress ) {
	this->register_address = _RegisterAddress;
}

void I2CDevice::SetRegisterValue( unsigned char _RegisterValue ) {
	this->register_value = _RegisterValue;
}

const char * I2CDevice::GetFilePath( ) {
	return this->device_bus_path;
}

int I2CDevice::GetDeviceFileHandle( ) {
	return this->file_handle;
}

int I2CDevice::ValidateBusId() throw(I2CSetupException&) {
	if ( this->bus_id > I2C_BUS_COUNT || this->bus_id < 1 ) {
		snprintf( this->err_message, sizeof( this->err_message ), "Bus ID : %d  is not a valid BUS for this device.", this->bus_id );
		throw( I2CSetupException( this->err_message ) );
	}
	else
		return EXIT_SUCCESS;
}

char * I2CDevice::ValidateBusPath( char * _I2CBusProposedPath ) throw( I2CSetupException& ) {
	if ( stat ( _I2CBusProposedPath, &buffer) == 0 )
		return _I2CBusProposedPath;
	else{
		snprintf( this->err_message, sizeof( this->err_message ), "Fatal I2C Error - Unable to locate the I2C Bus file : %s", _I2CBusProposedPath );
		throw I2CSetupException( this->err_message );
	}
}

short I2CDevice::GetValueFromRegister( unsigned char _RegisterAddress ) {
	if ( !this->device_initialized )
		throw I2CSetupException( "I2C Device Not Initialised (try 'my_device->InitDevice()')" );
	
	this->SetRegisterAddress( _RegisterAddress );
	this->write_buffer[ 0 ] = this->register_address;
	
	if ( write( this->GetDeviceFileHandle( ), this->write_buffer, 1 ) == 1 ) {
		return this->ReadDevice(1);
	}
	else {
		snprintf( this->err_message, sizeof( this->err_message ), "Fatal I2C Error - Unable to write to file : %s", this->GetFilePath( ));
		throw I2CSetupException( this->err_message );
	}
}

int I2CDevice::GetValuesFromRegisters(uint8_t *values_out, uint8_t start_register_address, uint8_t end_register_address) {
	
	for (int i = 0; i <= end_register_address - start_register_address; ++i) {
		values_out[i] = (uint8_t)this->GetValueFromRegister(start_register_address + i);
	}
	
	return end_register_address - start_register_address;
}

short I2CDevice::ReadDevice( size_t _BufferSize ) throw( I2CSetupException& ) {
	if(!this->device_initialized) throw I2CSetupException( "I2C Device Not Initialised ( try : 'obj->InitDevice()' )" );
	unsigned char buff[ _BufferSize ];
	if( read( this->GetDeviceFileHandle( ), buff, _BufferSize ) != _BufferSize ) {
		snprintf( this->err_message, sizeof( this->err_message ), "Fatal I2C Error - Unable to read from file : %s", this->GetFilePath( ) );
		throw I2CSetupException( this->err_message );
	}
	else
		return buff[ 0 ];
}

int I2CDevice::ConnectToDevice() {
	return ioctl(this->file_handle, I2C_SLAVE, this->device_address);
}

int I2CDevice::OpenDevice( ) throw( I2CSetupException& ) {
	this->file_handle = open( this->GetFilePath( ), O_RDWR );
	if( this->file_handle == 0 ) {
		snprintf( this->err_message, sizeof( this->err_message ), "Fatal I2C Error - Unable to open file : %s", this->GetFilePath( ) );
		throw I2CSetupException( this->err_message );
	}
	return this->file_handle;
}

int I2CDevice::WriteToDevice( size_t _BufferSize  ) throw( I2CSetupException& ) {
	if(!this->device_initialized) throw I2CSetupException( "I2C Device Not Initialised ( try : 'obj->InitDevice()' )" );
	try {
		if( _BufferSize > 1 ) {
			this->io_buffer[ 0 ] = this->register_address;
			this->io_buffer[ 1 ] = this->register_value;
			write( this->GetDeviceFileHandle( ), this->io_buffer, _BufferSize );
		}
		else {
			this->write_buffer[ 0 ] = this->register_address;
			write( this->GetDeviceFileHandle( ), this->write_buffer, _BufferSize );
		}
	}
	catch( exception& e ) {
		snprintf( this->err_message, sizeof( this->err_message ), "Fatal I2C Error - Unable to write to file : %s", this->GetFilePath( ) );
		throw I2CSetupException( this->err_message );
	}

	return EXIT_SUCCESS;
}
