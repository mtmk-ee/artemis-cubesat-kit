
// Internal headers
#include "utility/SimpleAgent.h"
#include "device/temp_sensors.h"
#include "device/switch.h"
#include "cubesat_defs.h"

// Standard headers
#include <iostream>
#include <fstream>
#include <unordered_map>

// The number of heaters available
#define NUM_HEATERS 1
// The interval between iterations in the run_agent() function
#define SLEEP_TIME 1

using namespace std;
using namespace cubesat;


// The agent object which allows for communication with COSMOS
SimpleAgent *agent;
Device *heater;
Device *temp_eps, *temp_obc, *temp_raspi, *temp_battery, *temp_pycubed;


// Other settings
float heater_enable_temp = 5; // Enable heater when temperatures are below this value (celcius)
float heater_disable_temp = 20; // Disable heater when temperatures are above this value (celcius)



// =========== Function Prototypes ===========


/**
 * @brief Sets up the heaters.
 */
void InitHeaters();
/**
 * @brief Checks the latest temperatures and handles activation of the heaters if necessary.
 */
void HandleTemperatures();
/**
 * @brief Retrieves the temperatures from agent_temp
 */
void GetTemperatures();
/**
 * @brief Switchs a heater on or off via agent_switch
 * @param enabled Set to true to enable a heater, or false to disable a heater
 */
void SetHeaterState(bool enabled);


//! Request to enable the heaters
string Request_Enable() {
	SetHeaterState(true);
	
	return "OK";
}
//! Request to disable the heaters
string Request_Disable() {
	SetHeaterState(false);
	
	return "OK";
}
// ===========================================



int main(int argc, char** argv) {
	// Create the agent
	agent = new SimpleAgent(CUBESAT_AGENT_HEATER_NAME, CUBESAT_NODE_NAME, true);
	agent->CrashIfNotOpen();
	agent->SetLoopPeriod(SLEEP_TIME);
	
	
	// Add heater
	heater = agent->NewDevice<Heater>("heater");
	heater->AddProperty<Heater::Enabled>(false);
	heater->AddProperty<Heater::UTC>(0);
	
	// Add temperature sensors and set default values
	temp_eps = agent->NewDevice<TemperatureSensor>("temp_eps");
	temp_obc = agent->NewDevice<TemperatureSensor>("temp_obc");
	temp_raspi = agent->NewDevice<TemperatureSensor>("temp_raspi");
	temp_battery = agent->NewDevice<TemperatureSensor>("temp_battery");
	temp_pycubed = agent->NewDevice<TemperatureSensor>("temp_pycubed");
	temp_eps->SetProperty<TemperatureSensor::Temperature>(273.15);
	temp_obc->SetProperty<TemperatureSensor::Temperature>(273.15);
	temp_raspi->SetProperty<TemperatureSensor::Temperature>(273.15);
	temp_battery->SetProperty<TemperatureSensor::Temperature>(273.15);
	temp_pycubed->SetProperty<TemperatureSensor::Temperature>(273.15);
	
	// Let the agent know all the devices have been set up
	agent->FinalizeDevices();
	
	// Add request callbacks
	agent->AddRequest({"enable", "on"}, Request_Enable, "Enables the heater", "Enables the heater", true);
	agent->AddRequest({"disable", "off"}, Request_Disable, "Disables the heater", "Disables the heater", true);
	
	// Debug print
	agent->DebugPrint();
	
	// Initialize the heaters
	InitHeaters();
	
	
	// Start executing the agent
	while ( agent->StartLoop() ) {
		
		// Update sensor readings
		GetTemperatures();
		
		// Act on temperature readings
		HandleTemperatures();
	}
	
	return 0;
}


void InitHeaters() {
	// TODO
}


void GetTemperatures() {
	static RemoteAgent agent_temp = agent->FindAgent(CUBESAT_AGENT_TEMP_NAME);
	
	// Attempt to reconnect to agent_temp
	if ( !agent_temp.Reconnect() ) {
		return;
	}
	
	// Get the temperature and timestamp values from agent_temp
	auto values = agent_temp.GetValues({
										   "device_tsen_temp_000", "device_tsen_utc_000",
										   "device_tsen_temp_001", "device_tsen_utc_001",
										   "device_tsen_temp_002", "device_tsen_utc_002",
										   "device_tsen_temp_003", "device_tsen_utc_003",
										   "device_tsen_temp_004", "device_tsen_utc_004"
									   });
	
	// Check if the values were not retrieved
	if ( values.empty() ) {
		printf("Failed to get temperatures from agent_temp\n");
		return;
	}
	
	// Store the temperatures
	temp_eps->SetProperty<TemperatureSensor::Temperature>(values["device_tsen_temp_000"].nvalue);
	temp_obc->SetProperty<TemperatureSensor::Temperature>(values["device_tsen_temp_001"].nvalue);
	temp_raspi->SetProperty<TemperatureSensor::Temperature>(values["device_tsen_temp_002"].nvalue);
	temp_battery->SetProperty<TemperatureSensor::Temperature>(values["device_tsen_temp_003"].nvalue);
	temp_pycubed->SetProperty<TemperatureSensor::Temperature>(values["device_tsen_temp_004"].nvalue);
	
	// Store the timestamps
	temp_eps->SetProperty<TemperatureSensor::UTC>(values["device_tsen_utc_000"].nvalue);
	temp_obc->SetProperty<TemperatureSensor::UTC>(values["device_tsen_utc_001"].nvalue);
	temp_raspi->SetProperty<TemperatureSensor::UTC>(values["device_tsen_utc_002"].nvalue);
	temp_battery->SetProperty<TemperatureSensor::UTC>(values["device_tsen_utc_003"].nvalue);
	temp_pycubed->SetProperty<TemperatureSensor::UTC>(values["device_tsen_utc_004"].nvalue);
}


void HandleTemperatures() {
	
	// TODO: Handle other temperature sensors. For now, only the battery temperature is used.
	
	// Get the battery temperature
	float temp = temp_battery->GetProperty<TemperatureSensor::Temperature>();
	
	// Handle temperature ranges
	// Temperature below value needed to enable heater
	if ( temp < heater_enable_temp ) {
		cout << "Temperature read from " << TEMPSENSOR_BATT_NAME << " is too low (" << temp << " C)" << endl;
		
		// Enable the heater
		SetHeaterState(true);
	}
	// Temperature above value needed to disable heater
	else if ( temp > heater_disable_temp ) {
		cout << "Temperature read from " << TEMPSENSOR_BATT_NAME << " is sufficiently high (" << temp << " C)" << endl;
		
		// Disable the heater
		SetHeaterState(false);
	}
	
	heater->Timestamp<Heater>();
}


void SetHeaterState(bool enabled) {
	static RemoteAgent agent_switch = agent->FindAgent(CUBESAT_AGENT_SWITCH_NAME);
	
	// Check if agent_switch has been connected to yet
	if ( !agent_switch.Reconnect() ) {
		return;
	}
	
	
	std::string request_str = enabled ? "enable" : "disable";
	
	cout << "Attempting to " << request_str << "heater" << endl;
	
	
	// Send a request to enable or disable the heater
	string response = agent_switch.SendRequest(request_str, SWITCH_HEATER_NAME);
	
	// Check if an error occurred
	if ( response.empty() ) {
		printf("Failed to send enable/disable request to agent_switch\n");
		return;
	}
	
	// Set the heater properties
	heater->Timestamp<Heater>();
	heater->SetProperty<Heater::Enabled>(enabled);
}


