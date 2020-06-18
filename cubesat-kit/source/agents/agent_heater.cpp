
// COSMOS headers
#include "agent/agentclass.h"
#include "support/configCosmos.h"
#include "agent/agentclass.h"
#include "support/jsonlib.h"

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

// Heartbeats of other agents
beatstruc beat_agent_temp;
beatstruc beat_agent_switch;

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
    
    
	
	// Retrieve the heartbeat from agents
	beat_agent_temp = agent->find_server(CUBESAT_NODE_NAME, CUBESAT_AGENT_TEMP_NAME, 2.0);
	beat_agent_switch = agent->find_server(CUBESAT_NODE_NAME, CUBESAT_AGENT_SWITCH_NAME, 2.0);
	
	// Make sure agent_temp has a heartbeat
	if ( !beat_agent_temp.exists ) {
		// Print an error and exit if no heartbeat was detected
		fprintf(agent->get_debug_fd(), "Failed to find agent_temp\n");
		agent->shutdown();
		exit(1);
	}
	
	// Make sure agent_switch has a heartbeat
	if ( !beat_agent_switch.exists ) {
		// Print an error and exit if no heartbeat was detected
		fprintf(agent->get_debug_fd(), "Failed to find agent_switch\n");
		agent->shutdown();
		exit(1);
	}
	
	
	// Add request callbacks (TODO)
	
	
	cout << "Successfully initialized agent" << endl;
}


void set_soh() {
	// TODO: Create the state of health string
    string soh = "{";
    soh += "}";
	

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
	// Time to wait for response from agent_temp
	const float kWaitSec = 2;
	
	string response;
	string request;
	
	// Update temperature readings for all sensors
	for (int i = 0; i < TEMPSENSOR_COUNT; ++i) {
		// Format the request to send
		request = "gettemp " + GetTempSensorName(i);
		
		// Request latest temperature for sensor with ID 'i'
		int ret = agent->send_request(beat_agent_temp, request, response, kWaitSec);
		
		// Check if an error occurred
		if ( ret < 0 ) {
			fprintf(agent->get_debug_fd(), "Failed to update temperature for sensor with name '%s': %s\n",
					GetTempSensorName(i).c_str(), cosmos_error_string(ret).c_str());
			continue;
		}
		
		// Store the temperature info for this sensor
		temp_info[i].temp = stof(response); // Convert reponse temperature from a string to a float
		temp_info[i].utc  = currentmjd(); // Probably should be timestamp for when temperature was originally read
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
	cout << "Attempting to " << (enabled ? "enable " : "disable ") << "heater" << endl;
	
	// Time to wait for agent_switch response
	const float kWaitSec = 2;
	
	// Create request string
	string request = std::string(enabled ? "enable " : "disable ") + SWITCH_HEATER_NAME;
	string response;
	
	// Request that agent_switch enables/disables the heater
	int ret = agent->send_request(beat_agent_switch, request, response, kWaitSec);
	
	// Check if an error occurred
	if ( ret < 0 ) {
		fprintf(agent->get_debug_fd(), "Failed to send enable/disable request to agent_switch: %s\n", cosmos_error_string(ret).c_str());
	}
	
}


int32_t request_list(){
	//list all available heaters and their states
	for ( i = 0; i < TEMPSENSOR_COUNT; ++i){
		cout << GetTempSensorName(i) << "Temperature: " << temp_info[i].tmp << '\n';
	}
}	


int32_t request_status(char *request, char* response, Agent *agent) {
	
