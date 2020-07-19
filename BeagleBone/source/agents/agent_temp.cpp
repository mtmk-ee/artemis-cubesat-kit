
// Internal headers
#include "utility/SimpleAgent.h"
#include "device/ADT7311.h"
#include "device/temp_sensors.h"

#include <iostream>
#include <fstream>

// The interval between iterations in the run_agent() function
#define SLEEP_TIME 1

using namespace std;
using namespace cubesat;

// The agent object which allows for communication with COSMOS
SimpleAgent *agent;
Device *temp_eps, *temp_obc, *temp_raspi, *temp_battery, *temp_pycubed;

ADT7311::Configuration default_sensor_config;
bool should_enable_sensors = true; // Whether or not the temperature sensors should be enabled
bool sensors_powered = false; // Whether or not the temperature sensors are actually enabled

struct DeviceAddress {
	uint8_t spi_bus;
	uint8_t dev_addr;
};
unordered_map<std::string, DeviceAddress> devices_addresses =  {
	{TEMPSENSOR_EPS_NAME, {0x00, 0x00}},
	{TEMPSENSOR_OBC_NAME, {0x00, 0x00}},
	{TEMPSENSOR_PAYLOAD_NAME, {0x00, 0x00}},
	{TEMPSENSOR_BATT_NAME, {0x00, 0x00}},
	{TEMPSENSOR_PYCUBED_NAME, {0x00, 0x00}},
};

//! Initializes the sensor devices
void InitSensors();
//! Attempts to connect a sensor
bool ConnectSensor(Device *sensor);
//! Wraps up communication with the sensor devices.
void DestroySensors();
//! Updates the temperature readings for a non-pycubed-connected sensor
void UpdateNormalTemperature(Device *sensor);
//! Grabs the latest readings from the sensor devices
void UpdateTemperatures();
//! Enables/disables power to sensors via agent_switch
void SetSensorPower(bool enabled);

//! Request for display data of a sensor
string Request_Sensor(vector<string> args);
//! Request for listing sensors
string Request_List();


int main(int argc, char** argv) {
	
	// Create the agent
	agent = new SimpleAgent(CUBESAT_AGENT_TEMP_NAME);
	agent->SetLoopPeriod(SLEEP_TIME);
	
	// Add the temperature sensor devices
	temp_eps = agent->NewDevice<TemperatureSensor>(TEMPSENSOR_EPS_NAME);
	temp_obc = agent->NewDevice<TemperatureSensor>(TEMPSENSOR_OBC_NAME);
	temp_battery = agent->NewDevice<TemperatureSensor>(TEMPSENSOR_PAYLOAD_NAME);
	temp_raspi = agent->NewDevice<TemperatureSensor>(TEMPSENSOR_BATT_NAME);
	temp_pycubed = agent->NewDevice<TemperatureSensor>(TEMPSENSOR_PYCUBED_NAME);
	
	temp_eps->AddProperty<TemperatureSensor::Temperature>(273.15);
	temp_obc->AddProperty<TemperatureSensor::Temperature>(273.15);
	temp_battery->AddProperty<TemperatureSensor::Temperature>(273.15);
	temp_raspi->AddProperty<TemperatureSensor::Temperature>(273.15);
	temp_pycubed->AddProperty<TemperatureSensor::Temperature>(273.15);
	
	temp_eps->AddProperty<TemperatureSensor::Voltage>(0.0f);
	temp_obc->AddProperty<TemperatureSensor::Voltage>(0.0f);
	temp_battery->AddProperty<TemperatureSensor::Voltage>(0.0f);
	temp_raspi->AddProperty<TemperatureSensor::Voltage>(0.0f);
	temp_pycubed->AddProperty<TemperatureSensor::Voltage>(0.0f);
	
	agent->FinalizeDevices();
	
	// Add request callbacks
	agent->AddRequest("sensor", Request_Sensor, "Returns the status of a sensor");
	agent->AddRequest("list", Request_List, "Returns a list of sensors");
	
	
	// Initialize the temperature sensors
	InitSensors();
	
	
	// Debug print
	agent->DebugPrint();
	
	// Start executing the agent
	while ( agent->StartLoop() ) {
		
		// Ensure correct power mode
		SetSensorPower(should_enable_sensors);
		
		// Update sensor readings
		UpdateTemperatures();
	}
	
	// Free sensor devices
	DestroySensors();
	
	// Free the agent
	delete agent;
	
	return 0;
}

bool ConnectSensor(Device *sensor) {
	
	sensor->Timestamp<TemperatureSensor>();
	
	// Get the device handler for this sensor
	ADT7311 *device = sensor->GetProperty<ADT7311*>("handler");
	
	// Check if the sensor is already connected
	if ( device->IsOpen() )
		return true;
	
	// We compare against the manufacturer ID to see if the sensors are connected
	const int kManufacturerID = 0b11000;
	
	// Check if the device is open by actually reading a register we know the value of
	if ( device->Open() >= 0 ) {
		device->ReadState();
		
		// Check if the manufacturer register has the correct value
		if ( device->GetManufacturerRegister().manufacturer_id != kManufacturerID ) {
			device->Close(); // Close the device to prevent dangling file descriptors
			
			sensor->SetProperty<TemperatureSensor::Enabled>(false);
			sensor->SetProperty<TemperatureSensor::Voltage>(0.0f);
			
			printf("Failed to open temperature sensor '%s' on SPI Bus %d at address %02x\n",
				   sensor->GetName().c_str(), device->GetBus(), device->GetDeviceAddr());
			
			return false;
		}
	}
	
	// Update the device properties
	sensor->SetProperty<TemperatureSensor::Enabled>(true);
	sensor->SetProperty<TemperatureSensor::Voltage>(3.3);
	
	printf("Successfully opened temperature sensor '%s'\n", sensor->GetName().c_str());
	
	return true;
}

void InitSensor(Device *sensor) {
	DeviceAddress config = devices_addresses[sensor->GetName()];
	
	// Create and store the device handler
	ADT7311 *device = new ADT7311(config.spi_bus, config.dev_addr);
	
	sensor->SetProperty<ADT7311*>("handler", device);
}

void InitSensors() {
	// Set a default configuration for the normal temperature sensors
	default_sensor_config.fault_queue = 0;
	default_sensor_config.ct_pin_polarity = 0;
	default_sensor_config.int_pin_polarity = 0;
	default_sensor_config.int_ct_mode = 0;
	default_sensor_config.operation_mode = 0;
	default_sensor_config.resolution = 0;
	
	InitSensor(temp_eps);
	InitSensor(temp_obc);
	InitSensor(temp_raspi);
}

void DestroySensors() {
	// Delete pointers to device handlers
	delete temp_eps->GetProperty<ADT7311*>("handler");
	delete temp_obc->GetProperty<ADT7311*>("handler");
	delete temp_raspi->GetProperty<ADT7311*>("handler");
}

void UpdateNormalTemperature(Device *sensor) {
	
	
	// Reconnect to the sensor
	ConnectSensor(sensor);
	
	// Get the device handler
	ADT7311 *device = sensor->GetProperty<ADT7311*>("handler");
	
	// Update readings if the device is open
	if ( device->IsOpen() ) {
		
		// Update readings from the device
		device->ReadState();
		
		// Update the temperature reading
		sensor->Timestamp<TemperatureSensor>();
		sensor->SetProperty<TemperatureSensor::Temperature>(device->GetTemperature());
	}
}

void UpdateTemperatures() {
	
	UpdateNormalTemperature(temp_eps);
	UpdateNormalTemperature(temp_obc);
	UpdateNormalTemperature(temp_raspi);
	
	static RemoteAgent agent_pycubed = agent->FindAgent(CUBESAT_AGENT_PYCUBED_NAME);
	
	if ( agent_pycubed.Reconnect() ) {
		
		auto values = agent_pycubed.GetValues({"device_batt_temp_000", "device_imu_temp_000"});
		
		// Check if the request failed
		if ( values.empty() )
			return;
		
		temp_battery->SetProperty<TemperatureSensor::Temperature>(values["device_batt_temp_000"].nvalue);
		temp_pycubed->SetProperty<TemperatureSensor::Temperature>(values["device_batt_imu_000"].nvalue);
		temp_battery->Timestamp<TemperatureSensor>();
		temp_pycubed->Timestamp<TemperatureSensor>();
		
	}

}

void SetSensorPower(bool enable) {
	should_enable_sensors = enable;
	
	static RemoteAgent agent_switch = agent->FindAgent(CUBESAT_AGENT_SWITCH_NAME);
	
	if ( agent_switch.Reconnect() ) {
		// Request to enable/disable power to temperature sensor switch
		string response = agent_switch.SendRequest("enable", "sw_temp");
		
		// Check if the request was successful
		if ( response.find("NOK") == string::npos )
			sensors_powered = enable;
		
		// If they are disabled, make sure the sensors are closed
		if ( !sensors_powered ) {
			temp_eps->GetProperty<ADT7311*>("handler")->Close();
			temp_battery->GetProperty<ADT7311*>("handler")->Close();
			temp_raspi->GetProperty<ADT7311*>("handler")->Close();
		}
	}
}

string Request_Sensor(vector<string> args) {
	// Check if the device name is valid
	if ( !agent->DeviceExists(args[0]) )
		return "No matching sensor";
	
	Device *device = agent->GetDevice(args[0]);
	ADT7311 *handler = device->GetProperty<ADT7311*>("handler");
	
	// Generate the response
	stringstream ss;
	ss << "{";
	ss <<	"\"utc\": " << device->GetProperty<TemperatureSensor::UTC>() << ", ";
	ss <<	"\"temp\": " << device->GetProperty<TemperatureSensor::Temperature>() << ", ";
	ss <<	"\"spi_bus\": " << (handler != nullptr ? std::to_string(handler->GetBus()) : "N/A") << ", ";
	ss <<	"\"address\": " << (handler != nullptr ? std::to_string(handler->GetDeviceAddr()) : "N/A") << ", ";
	ss <<	"\"enabled\": " << (handler != nullptr ? std::to_string(handler->IsOpen()) : "true");
	ss << "}";
	
	return ss.str();
}

string Request_List() {
	
	stringstream ss;
	
	// Use a convenient lambda function to add sensor information
	auto add_sensor_info = [&ss](Device *sensor) {
		ADT7311 *device = sensor->GetProperty<ADT7311*>("handler");
		
		ss <<	"\"" << sensor->GetName() << "\": {";
		ss <<		"\"utc\": " << sensor->GetProperty<TemperatureSensor::UTC>() << ", ";
		ss <<		"\"temp\": " << sensor->GetProperty<TemperatureSensor::Temperature>() << ",";
		ss <<		"\"spi_bus\": " << (device != nullptr ? std::to_string(device->GetBus()) : "N/A") << ", ";
		ss <<		"\"address\": " << (device != nullptr ? std::to_string(device->GetDeviceAddr()) : "N/A") << ", ";
		ss <<		"\"enabled\": " << (device != nullptr ? std::to_string(device->IsOpen()) : "true");
		
		ss <<	"},";
	};
	
	ss << "{";
	add_sensor_info(temp_eps);
	add_sensor_info(temp_obc);
	add_sensor_info(temp_raspi);
	add_sensor_info(temp_battery);
	add_sensor_info(temp_pycubed);
	ss << "}";
	
	return ss.str();
}
