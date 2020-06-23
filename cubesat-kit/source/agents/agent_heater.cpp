
// COSMOS headers
#include "agent/agentclass.h"
#include "support/configCosmos.h"
#include "agent/agentclass.h"
#include "support/jsonlib.h"
#include "support/jsonclass.h"

// Internal headers
#include "device/temp_sensors.h"
#include "device/switched_devices.h"
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
Agent *agent;
int heater_cindex;


// Other settings
float heater_enable_temp = 5; // Enable heater when temperatures are below this value (celcius)
float heater_disable_temp = 20; // Disable heater when temperatures are above this value (celcius)


// A convenient struct for storing temperature sensor info retreived from agent_temp
struct {
	float temp; // The temperature in celcius
	double utc; // Timestamp of this reading
} temp_info[TEMPSENSOR_COUNT];




// =========== Function Prototypes ===========

/**
 * @brief Sets up the agent. Prints a message and exits with code 1 if an error occurs during setup.
 */
void init_agent();
/**
 * @brief Sets the SOH string for the agent.
 */
void set_soh();
/**
 * @brief Main loop for this agent.
 */
void run_agent();


/**
 * @brief Sets up the heaters.
 */
void init_heaters();
/**
 * @brief Checks the latest temperatures and handles activation of the heaters if necessary.
 */
void handle_temps();
/**
 * @brief Retrieves the temperatures from agent_temp
 */
void get_temps();
/**
 * @brief Switchs a heater on or off via agent_switch
 * @param enabled Set to true to enable a heater, or false to disable a heater
 */
void set_heater_state(bool enabled);
// ===========================================



int main(int argc, char** argv) {
	
	// Initialize the agent
	init_agent();
	
	// Initialize the heaters
	init_heaters();
	
	// Set the state of health string for this agent
	set_soh();
	
	// Run the main loop for this agent
	run_agent();
	
	
	return 0;
}

void init_agent() {
	// Create the agent
	agent = new Agent(CUBESAT_NODE_NAME, CUBESAT_AGENT_HEATER_NAME);
	
	// Make sure the agent initialized successfully
	if ( agent->cinfo == nullptr || !agent->running() ) {
		
		// Notify that an error occurred and exit
		printf("Failed to open [%s:%s]\n", CUBESAT_NODE_NAME, CUBESAT_AGENT_HEATER_NAME);
		exit(1);
	}
	
	
	// Add heater
	int status = json_addpiece(agent->cinfo, "htr", (uint16_t)DeviceType::HTR);
	if ( status < 0 ) {
		fprintf(agent->get_debug_fd(), "Failed to add heater %s\n", cosmos_error_string(status).c_str());
		agent->shutdown();
		exit(1);
	}
	heater_cindex = agent->cinfo->pieces[status].cidx;
	agent->cinfo->device[heater_cindex].htr.enabled = false;
	
	
	
	// Add request callbacks (TODO)
	
	
	cout << "Successfully initialized agent" << endl;
}


void set_soh() {
	// TODO: Create the state of health string
	string soh = "{\"device_htr_enabled_000\"}";
	
	// Set the SOH string
	agent->set_sohstring(soh);
}

void run_agent() {
	
	// Start executing the agent
	while ( agent->running() ) {
		
		// Update sensor readings
		get_temps();
		
		// Act on temperature readings
		handle_temps();
		
		// Pause thread execution for a bit
		COSMOS_SLEEP(SLEEP_TIME);
	}
	
}

void init_heaters() {
	// TODO
}


void get_temps() {
	static beatstruc temp_beat;
	
	
	// Locate agent_temp if not present
	if ( temp_beat.utc == 0. ) {
		temp_beat = agent->find_server(CUBESAT_NODE_NAME, CUBESAT_AGENT_TEMP_NAME, 5.);
		
		if ( temp_beat.utc == 0. ) {
			return;
		}
	}
	
	
	Json jresult;
	string response;
	
	
	agent->send_request(temp_beat, "getvalue {\"device_tsen_temp000\", \"device_tsen_temp001\", \"device_tsen_temp002\"}, \"device_tsen_temp003\", \"device_tsen_temp004\"", response);
	int status = jresult.extract_object(response);
	
	if ( status > 0 ) {
		
		// Search the response JSON for corresponding data
		for (Json::Member member : jresult.Members) {
			if ( member.value.name.find("tsen_temp000") != string::npos ) {
				temp_info[0].temp = member.value.nvalue;
				temp_info[0].utc  = currentmjd();
			}
			else if ( member.value.name.find("tsen_temp001") != string::npos ) {
				temp_info[1].temp = member.value.nvalue;
				temp_info[1].utc  = currentmjd();
			}
			else if ( member.value.name.find("tsen_temp002") != string::npos ) {
				temp_info[2].temp = member.value.nvalue;
				temp_info[2].utc  = currentmjd();
			}
			else if ( member.value.name.find("tsen_temp003") != string::npos ) {
				temp_info[3].temp = member.value.nvalue;
				temp_info[3].utc  = currentmjd();
			}
			else if ( member.value.name.find("tsen_temp004") != string::npos ) {
				temp_info[4].temp = member.value.nvalue;
				temp_info[4].utc  = currentmjd();
			}
		}
	}
}


void handle_temps() {
	
	// TODO: Handle other temperature sensors. For now, only the battery temperature is used.
	
	float temp = temp_info[TEMPSENSOR_BATT_ID].temp;
	
	// Handle temperature ranges
	// Temperature below value needed to enable heater
	if ( temp < heater_enable_temp ) {
		cout << "Temperature read from " << TEMPSENSOR_BATT_NAME << " is too low (" << temp << " C)" << endl;
		
		// Enable the heater
		set_heater_state(true);
	}
	// Temperature above value needed to disable heater
	else if ( temp > heater_disable_temp ) {
		cout << "Temperature read from " << TEMPSENSOR_BATT_NAME << " is sufficiently high (" << temp << " C)" << endl;
		
		// Disable the heater
		set_heater_state(false);
	}
	
}


void set_heater_state(bool enabled) {
	static beatstruc switch_beat;
	
	
	cout << "Attempting to " << (enabled ? "enable " : "disable ") << "heater" << endl;
	
	
	
	// Locate agent_switch if not present
	if ( switch_beat.utc == 0. ) {
		switch_beat = agent->find_server(CUBESAT_NODE_NAME, CUBESAT_AGENT_SWITCH_NAME, 5.);
		if ( switch_beat.utc == 0. ) {
			fprintf(agent->get_debug_fd(), "Failed to find agent_switch\n");
			return;
		}
	}
	
	// Create request string
	string request = std::string(enabled ? "enable " : "disable ") + "heater";
	string response;
	
	// Send request
	int status = agent->send_request(switch_beat, request, response);
	
	// Check if an error occurred
	if ( status < 0 ) {
		fprintf(agent->get_debug_fd(), "Failed to send enable/disable request to agent_switch: %s\n", cosmos_error_string(status).c_str());
	}
	
	agent->cinfo->device[heater_cindex].htr.enabled = enabled;
	
}


