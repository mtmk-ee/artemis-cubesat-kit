
// COSMOS headers
#include "agent/agentclass.h"
#include "support/configCosmos.h"
#include "agent/agentclass.h"
#include "support/jsonlib.h"

// Internal headers
#include "cubesat_defs.h"
#include "device/switch.h"
#include "device/switched_devices.h"

// Standard headers
#include <iostream>
#include <fstream>

// The interval between iterations in the run_agent() function
#define SLEEP_TIME 1

using namespace std;
using namespace cubesat;


// A convenient struct for storing switch information
struct SwitchInfo {
	Switch *sw; // The switch device
	
	int dindex; // COSMOS device index
	int cindex; // COSMOS component index
	int pindex; // COSMOS piece index
} switches[SWITCH_COUNT]; // Array of SwitchInfo objects, one for each controllable switch


// The agent object which allows for communication with COSMOS
Agent *agent;

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
 * @brief Initializes all the switches needed
 */
void init_switches();


/**
 * @brief Attempts to set the state of a switch (on/off)
 * @param switch_id The switch ID number. See the file device/switched_devices.h for more info
 * @param enabled True for on, false for off
 */
void set_switch_state(int switch_id, bool enabled);


// Request callbacks
int32_t request_enable(char *request, char* response, Agent *agent);
int32_t request_disable(char *request, char* response, Agent *agent);
int32_t request_status(char *request, char* response, Agent *agent);
int32_t request_list(char *request, char* response, Agent *agent);
// ===========================================






int main(int argc, char** argv) {
	
	// Initialize the agent
    init_agent();
	
	// Set the state of health string for this agent
	set_soh();
	
	// Initialize the switches
    init_switches();

	// Run the main loop for this agent
    run_agent();


    return 0;
}

/**
 * @brief init_agent Sets up the agent. Prints a message and exits with code 1 if an error occurs.
 */
void init_agent() {
    // Create the agent
    agent = new Agent(CUBESAT_NODE_NAME, CUBESAT_AGENT_SWITCH_NAME);

    if ( agent->cinfo == nullptr || !agent->running() ) {
        printf("Failed to open [%s:%s]\n", CUBESAT_NODE_NAME, CUBESAT_AGENT_SWITCH_NAME);
        exit (1);
    }


    
    // Add request callbacks
    int status;
    
    if ( (status = agent->add_request("enable", request_enable)) )
        exit (status);
    
    if ( (status = agent->add_request("disable", request_disable)) )
        exit (status);
	
	if ( (status = agent->add_request("status", request_status)) )
        exit (status);
	
	if ( (status = agent->add_request("list", request_list)) )
        exit (status);
	
	
	cout << "Successfully initialized agent" << endl;
}

void set_soh() {
	
	// TODO: Create the state of health string
    string soh = "{";
    soh.append("}");
	
    // set the soh string
    agent->set_sohstring(soh);
}

/**
 * @brief run_agent Runs the main loop for this agent.
 */
void run_agent() {

    // Start executing the agent
    while ( agent->running() ) {

        

        // Sleep for a bit
        COSMOS_SLEEP(SLEEP_TIME);
    }

}

void add_switch_piece(int switch_id) {
	
	// Convert the switch ID to its name
	const std::string &name = GetSwitchName(switch_id);
	
	// Try adding a piece to the COSMOS namespace
	int pindex = json_addpiece(agent->cinfo, name, (uint16_t)DeviceType::SWCH);
	
	// Check if an error occurred
	if ( pindex < 0 ) {
		// Print error message and exit
		fprintf(agent->get_debug_fd(), "Failed to add switch '%s': %s\n", name.c_str(), cosmos_error_string(pindex).c_str());
		agent->shutdown();
		exit(1);
	}
	
	// Store the COSMOS namespace indices for future reference
	int cindex = agent->cinfo->pieces[pindex].cidx;
	int dindex = agent->cinfo->device[cindex].all.didx;
	
	switches[switch_id].pindex = pindex;
	switches[switch_id].cindex = cindex;
	switches[switch_id].dindex = dindex;
}

/**
 * @brief init_sensors Sets up the sensors for reading
 */
void init_switches() {
	
	// Add all the switch pieces to the COSMOS namespace
	for (int i = 0; i < SWITCH_COUNT; ++i)
		add_switch_piece(i);
	
	
	// TODO: determine switch pins
	
	// Add heater switch
	switches[SWITCH_HEATER_ID].sw = new Switch();
	
	// Add temperature sensor switch
	switches[SWITCH_TEMP_ID].sw = new Switch();
	
	// Add sun sensor switch
	switches[SWITCH_SS_ID].sw = new Switch();
	
	// Add payload switch
	switches[SWITCH_PAYLOAD_ID].sw = new Switch();
	
	
}

void set_switch_state(int switch_id, bool enabled) {
	
	// Make sure the switch ID is valid
	if ( switch_id < 0 || switch_id >= SWITCH_COUNT ) {
		// TODO: handle error
		return;
	}
	
	// Pull the corresponding SwitchInfo object according the given ID
	SwitchInfo &swinfo = switches[switch_id];
	
	// Turn the switch on or off
	if ( enabled )
		swinfo.sw->SetHigh();
	else
		swinfo.sw->SetLow();
}



/**
 * @brief Enable device request
 * @return 
 */
int32_t request_enable(char *request, char* response, Agent *agent) {
	
	// Strip out the switch name from the request string
    char switch_name[128];
    int status = sscanf(request, "%*s %s", switch_name);
    
    
    // Check if the given request is invalid
    if ( status != 1 ) {
		
		// Print an error message
        sprintf(response, "Usage: enable <name>");
        return 1;
    }
	
	// Check if 'name' is 'all', in which case all switches should be turned on
	if ( strcmp(switch_name, "all") == 0 ) {
		
		// Enable all switches
		for (int i = 0; i < SWITCH_COUNT; ++i) {
			set_switch_state(i, true);
		}
		
		// Print success message
		sprintf(response, "OK");
		return 0;
	}
	
	// Convert the switch name to its corresponding ID
	int switch_id = GetSwitchID(switch_name);
	
	// Check if a switch with the given name exists
	if ( switch_id < 0 ) {
		// Print an error message
		sprintf(response, "No switch with the name '%s' exists.\n", switch_name);
		return 1;
	}
	
	// Enable the switch
	set_switch_state(switch_id, true);
	
    // Indicate success
	sprintf(response, "OK");
    return 0;
}

int32_t request_disable(char *request, char* response, Agent *agent) {
	// Strip out the switch name from the request string
	char switch_name[128];
    int status = sscanf(request, "%*s %s", switch_name);
    
    
    // Check if the given request is invalid
    if ( status != 1 ) {
		// Print an error message
        sprintf(response, "Usage: disable <name>");
        return 1;
    }
	
	// Check if 'name' is 'all', in which case all switches should be disabled
	if ( strcmp(switch_name, "all") == 0 ) {
		
		// Disable all switches
		for (int i = 0; i < SWITCH_COUNT; ++i) {
			set_switch_state(i, false);
		}
		
		// Print success message
		sprintf(response, "OK");
		return 0;
	}
	
	// Convert the switch name to its corresponding ID
	int switch_id = GetSwitchID(switch_name);
	
	// Check if a switch with the given name exists
	if ( switch_id < 0 ) {
		// Print an error message
		sprintf(response, "No switch with the name '%s' exists.\n", switch_name);
		return 1;
	}
	
	// Disable the switch
	set_switch_state(switch_id, false);
	
	// Indicate success
	sprintf(response, "OK");
    return 0;
}

int32_t request_status(char *request, char* response, Agent *agent) {
	// Strip out the switch name from the request string
	char switch_name[128];
    int status = sscanf(request, "%*s %s", switch_name);
    
    
    // Check if the given request is invalid
    if ( status != 1 ) {
	// Print an error message
        sprintf(response, "Usage: disable <name>");
        return 1;
    }
   
	// Convert the switch name to its corresponding ID
	int switch_id = GetSwitchID(switch_name);
	
	// Check if a switch with the given name exists
	if ( switch_id < 0 ) {
		// Print an error message
		sprintf(response, "No switch with the name '%s' exists.\n", switch_name);
		return 1;
	}
	//Returns enabled if switch_id is true
	if ( switch_id == true ){
		return enabled
	}
	//Returns disbles if switch_id is false
	if ( switch_id == false ){
		return disabled
	}
	return 0;
}
int32_t request_list(char *request, char* response, Agent *agent) {
	// Strip out the switch name from the request string
	char switch_name[128];
    int status = sscanf(request, "%*s %s", switch_name);
    
    
    // Check if the given request is invalid
    if ( status != 1 ) {
		// Print an error message
        sprintf(response, "Usage: disable <name>");
        return 1;
    }
	
	// Check if 'name' is 'all'
	if ( strcmp(switch_name, "all") == 0 ) {
		
		// list all available switches and states
		for (int i = 0; i < SWITCH_COUNT; ++i) {
		    //if( switches[i] == true ){
			//need to return name
			switches[i].stat = request_status(switches[i]) //supposed to be returning switch status
		}

	}	
	
	return 0;
}
