
// Internal headers
#include "utility/SimpleAgent.h"
#include "device/OPT3001.h"
#include "device/sun_sensors.h"

#include <iostream>
#include <fstream>
#include <unordered_map>

// The interval between iterations in the run_agent() function
#define SLEEP_TIME 1

using namespace std;
using namespace cubesat;


// The agent object which allows for communication with COSMOS
SimpleAgent *agent;


// A convenient struct for storing sun sensor information
Device *plus_x, *minus_x;
Device *plus_y, *minus_y;
Device *plus_z, *minus_z;
OPT3001::Configuration default_sensor_config;
bool should_enable_sensors = true; // Whether or not the sensors should be enabled
bool sensors_powered = false; // Whether or not the sensors are actually enabled

struct DeviceAddress {
	uint8_t i2c_bus;
	uint8_t dev_addr;
};

unordered_map<std::string, DeviceAddress> devices_addresses =  {
	{SUNSENSOR_PLUSX_NAME, {0x00, 0x00}},
	{SUNSENSOR_MINUSX_NAME, {0x00, 0x00}},
	{SUNSENSOR_PLUSY_NAME, {0x00, 0x00}},
	{SUNSENSOR_MINUSY_NAME, {0x00, 0x00}},
	{SUNSENSOR_PLUSZ_NAME, {0x00, 0x00}},
	{SUNSENSOR_MINUSZ_NAME, {0x00, 0x00}},
};

// =========== Function Prototypes ===========


//! Initializes the sensor devices
void InitSensors();
//! Attempts to connect a sensor
bool ConnectSensor(Device *sensor);
//! Wraps up communication with the sensor devices.
void DestroySensors();
//! Grabs the latest readings from the sensor devices
void UpdateSensors();
//! Enables/disables power to sensors via agent_switch
void SetSensorPower(bool enabled);

/**
 * @brief Grabs the latest readings from the sensor devices
 */
void update_readings();

// Request callbacks
int32_t request_sensor(char *request, char* response, Agent *agent);
int32_t request_list(char *request, char* response, Agent *agent);

string Request_Sensor(vector<string> args);
string Request_List();


// ===========================================







int main(int argc, char** argv) {
	
	// Create the agent
	agent = new SimpleAgent(CUBESAT_AGENT_SUNSENSOR_NAME);
	agent->SetLoopPeriod(SLEEP_TIME);
	
	// Add the sun sensor devices
	plus_x = agent->NewDevice<SunSensor>(SUNSENSOR_PLUSX_NAME);
	minus_x = agent->NewDevice<SunSensor>(SUNSENSOR_MINUSX_NAME);
	plus_y = agent->NewDevice<SunSensor>(SUNSENSOR_PLUSY_NAME);
	minus_y = agent->NewDevice<SunSensor>(SUNSENSOR_MINUSY_NAME);
	plus_z = agent->NewDevice<SunSensor>(SUNSENSOR_PLUSZ_NAME);
	minus_z = agent->NewDevice<SunSensor>(SUNSENSOR_MINUSZ_NAME);
	
	plus_x->AddProperty<SunSensor::Temperature>(0);
	minus_x->AddProperty<SunSensor::Temperature>(0);
	plus_y->AddProperty<SunSensor::Temperature>(0);
	minus_y->AddProperty<SunSensor::Temperature>(0);
	plus_z->AddProperty<SunSensor::Temperature>(0);
	minus_z->AddProperty<SunSensor::Temperature>(0);
	
	plus_x->AddProperty<SunSensor::UTC>(0);
	minus_x->AddProperty<SunSensor::UTC>(0);
	plus_y->AddProperty<SunSensor::UTC>(0);
	minus_y->AddProperty<SunSensor::UTC>(0);
	plus_z->AddProperty<SunSensor::UTC>(0);
	minus_z->AddProperty<SunSensor::UTC>(0);
	
	agent->FinalizeDevices();
	
	agent->AddRequest("sensor", Request_Sensor, "Returns the status of a sensor");
	agent->AddRequest("list", Request_List, "Returns a list of sensors");
	
	// Initialize the sun sensors
	InitSensors();
	
	// Run the main loop for this agent
	while ( agent->StartLoop() ) {
		
		
		// Ensure correct power mode
		SetSensorPower(should_enable_sensors);
		
		// Update sensor readings
		UpdateSensors();
		
	}
	
	DestroySensors();
	delete agent;
	
	return 0;
}

void InitSensor(Device *sensor) {
	DeviceAddress config = devices_addresses[sensor->GetName()];
	OPT3001 *device = new OPT3001(config.i2c_bus, config.dev_addr);
	sensor->SetProperty<OPT3001*>("handler", device);
}


void InitSensors() {
	// Set default configuration values
	default_sensor_config.RangeNumber = 0b1100; // This is the only available mode
	default_sensor_config.ConversionTime = 1; // 0 = 100ms, 1 = 800 ms
	default_sensor_config.ModeOfConversionOperation = 0; // 00 = Shutdown, 01 = Single-shot, 10,11 = continuous conversion
	default_sensor_config.Latch = 1;
	default_sensor_config.Polarity = 0; // 0 = INT pin active low, 1 = INT pin active high
	default_sensor_config.MaskExponent = 0;
	default_sensor_config.FaultCount = 0; // 00 = One fault, 01 = two faults, 10 = four faults, 11 = eight faults
	
	// Initialize each sensor
	InitSensor(plus_x);
	InitSensor(minus_x);
	InitSensor(plus_y);
	InitSensor(minus_y);
	InitSensor(plus_z);
	InitSensor(minus_z);
}

bool ConnectSensor(Device *sensor) {
	
	sensor->Timestamp<SunSensor>();
	OPT3001 *device = sensor->GetProperty<OPT3001*>("handler");
	
	// Check if the sensor is already connected
	if ( device->IsOpen() )
		return true;
	
	
	// Attempt to open the device
	if ( device->Open() < 0 ) {
		printf("Failed to open sun sensor '%s' on I2C Bus %d at address %02x\n",
			   sensor->GetName().c_str(),
			   device->GetBusAddr(), device->GetDeviceAddr());
		
		device->Close();
		
		sensor->SetProperty<SunSensor::Voltage>(0.0);
		
		return false;
	}
	else {
		
		// Set sun sensor configuration
		device->SetConfiguration(default_sensor_config);
		sensor->SetProperty<SunSensor::Voltage>(3.3);
		
		printf("Successfully opened sun sensor '%s'\n", sensor->GetName().c_str());
		
		return true;
	}
}

void DestroySensors() {
	delete plus_x->GetProperty<OPT3001*>("handler");
	delete minus_x->GetProperty<OPT3001*>("handler");
	delete plus_y->GetProperty<OPT3001*>("handler");
	delete minus_y->GetProperty<OPT3001*>("handler");
	delete plus_z->GetProperty<OPT3001*>("handler");
	delete minus_z->GetProperty<OPT3001*>("handler");
}

void UpdateSensor(Device *sensor) {
	ConnectSensor(sensor);
	float new_lux = 0;
	
	OPT3001 *device = sensor->GetProperty<OPT3001*>("handler");
	if ( device->IsOpen() ) {
		device->ReadState();
		
		// Update the device properties
		sensor->Timestamp<SunSensor>();
		sensor->SetProperty<SunSensor::Temperature>(device->GetLux());
	}
	
}
 
void UpdateSensors() {
	UpdateSensor(plus_x);
	UpdateSensor(minus_x);
	UpdateSensor(plus_y);
	UpdateSensor(minus_y);
	UpdateSensor(plus_z);
	UpdateSensor(minus_z);
}

void SetSensorPower(bool enable) {
	should_enable_sensors = enable;
	
	static RemoteAgent agent_switch = agent->FindAgent(CUBESAT_AGENT_SWITCH_NAME);
	
	if ( agent_switch.Reconnect() ) {
		// Request to enable/disable power to sun sensor switch
		string response = agent_switch.SendRequest("enable", "sw_ss");
		
		// Check if the request was successful
		if ( response.find("NOK") == string::npos )
			sensors_powered = enable;
		
		// If they are disabled, make sure the sensors are closed
		if ( !sensors_powered ) {
			plus_x->GetProperty<OPT3001*>("handler")->Close();
			minus_x->GetProperty<OPT3001*>("handler")->Close();
			plus_y->GetProperty<OPT3001*>("handler")->Close();
			minus_y->GetProperty<OPT3001*>("handler")->Close();
			plus_z->GetProperty<OPT3001*>("handler")->Close();
			minus_z->GetProperty<OPT3001*>("handler")->Close();
		}
	}
}

string Request_Sensor(vector<string> args) {
	// Check if the device name is valid
	if ( !agent->DeviceExists(args[0]) )
		return "No matching sensor";
	
	Device *device = agent->GetDevice(args[0]);
	OPT3001 *handler = device->GetProperty<OPT3001*>("handler");
	
	// Generate the response
	stringstream ss;
	ss << "{";
	ss <<	"\"utc\": " << device->GetProperty<SunSensor::UTC>() << ", ";
	ss <<	"\"lux\": " << device->GetProperty<SunSensor::Temperature>() << ", ";
	ss <<	"\"i2c_bus\": " << (handler != nullptr ? std::to_string(handler->GetBusAddr()) : "N/A") << ", ";
	ss <<	"\"address\": " << (handler != nullptr ? std::to_string(handler->GetDeviceAddr()) : "N/A") << ", ";
	ss <<	"\"enabled\": " << (handler != nullptr ? std::to_string(handler->IsOpen()) : "true");
	ss << "}";
	
	return ss.str();
}

string Request_List() {
	
	stringstream ss;
	
	// Use a convenient lambda function to add sensor information
	auto add_sensor_info = [&ss](Device *sensor) {
		OPT3001 *device = sensor->GetProperty<OPT3001*>("handler");
		
		ss <<	"\"" << sensor->GetName() << "\": {";
		ss <<		"\"utc\": " << sensor->GetProperty<SunSensor::UTC>() << ", ";
		ss <<		"\"lux\": " << sensor->GetProperty<SunSensor::Temperature>() << ",";
		ss <<		"\"spi_bus\": " << (device != nullptr ? std::to_string(device->GetBusAddr()) : "N/A") << ", ";
		ss <<		"\"address\": " << (device != nullptr ? std::to_string(device->GetDeviceAddr()) : "N/A") << ", ";
		ss <<		"\"enabled\": " << (device != nullptr ? std::to_string(device->IsOpen()) : "true");
		
		ss <<	"},";
	};
	
	ss << "{";
	add_sensor_info(plus_x);
	add_sensor_info(minus_x);
	add_sensor_info(plus_y);
	add_sensor_info(minus_y);
	add_sensor_info(plus_z);
	add_sensor_info(minus_z);
	ss << "}";
	
	return ss.str();
}
