
#include "device/OPT3001.h"

#include <cmath>


using namespace std;
using namespace cubesat;


OPT3001::OPT3001() {
	// This should be removed later on
}
OPT3001::OPT3001(uint8_t bus, uint8_t device) : I2CDevice(bus, device) {
	
}
OPT3001::~OPT3001() {
	
}

void OPT3001::ReadState() {
	
	// Read result register
	result.raw_data = ReadRegister((uint8_t)Register::Result);
	
	// Read configuration register
	config.raw_data = ReadRegister((uint8_t)Register::Config);
	
	// Read low limit register
	low_limit.raw_data = ReadRegister((uint8_t)Register::LowLimit);
	
	// Read high limit register
	high_limit.raw_data = ReadRegister((uint8_t)Register::HighLimit);
	
}

bool OPT3001::SetConfiguration(Configuration config) {
	// Should we do 'this->config = config'? Maybe, but maybe the state of the device
	// since the last call to ReadState() should be kept instead
	return WriteRegister((uint8_t)Register::Config, config.raw_data) == 0;
}

float OPT3001::GetLux() const {
	// Convert the temperature from binary values to decimal
	return 0.01f * pow(2, result.exponent) * result.result;
}

float OPT3001::GetHighLimit() const {
	// Convert the temperature from binary values to decimal
	return 0.01f * pow(2, high_limit.exponent) * high_limit.result;
}

float OPT3001::GetLowLimit() const {
	// Convert the temperature from binary values to decimal
	return 0.01f * pow(2, low_limit.exponent) * low_limit.result;
}

