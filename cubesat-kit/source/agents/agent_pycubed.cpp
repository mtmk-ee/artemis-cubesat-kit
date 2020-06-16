
// COSMOS headers
#include "agent/agentclass.h"
#include "support/configCosmos.h"
#include "agent/agentclass.h"
#include "support/jsonlib.h"

// Internal headers
#include "cubesat_defs.h"
#include "device/PyCubed.h"

// Standard headers
#include <iostream>
#include <fstream>
#include <unordered_map>

// The interval between iterations in the run_agent() function
#define SLEEP_TIME 1

using namespace std;
using namespace cubesat;


// The agent object which allows for communication with COSMOS
Agent *agent;


PyCubed *pycubed; // The PyCubed device handler

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
 * @brief Sets up the pycubed.
 */
void init_pycubed();
/**
 * @brief Handles operations of the PyCubed
 */
void handle_pycubed();
/**
 * @brief Callback function for received files from PyCubed.
 * @param name The file location
 */
void file_received(const std::string &name);

// ===========================================









int main(int argc, char** argv) {
	
	// Initialize the agent
    init_agent();
	
	// Initialize the pycubed
    init_pycubed();

	// Run the main loop for this agent
    run_agent();


    return 0;
}


void init_agent() {
    // Create the agent
    agent = new Agent(CUBESAT_NODE_NAME, CUBESAT_AGENT_PYCUBED_NAME);

	// Make sure the agent initialized successfully
    if ( agent->cinfo == nullptr || !agent->running() ) {
		
		// Notify that an error occurred and exit
        printf("Failed to open [%s:%s]\n", CUBESAT_NODE_NAME, CUBESAT_AGENT_PYCUBED_NAME);
        exit (1);
    }

    
    // Add request callbacks (TODO)
	
	
	cout << "Successfully initialized agent" << endl;
}

void set_soh() {
	// Create the state of health string
    string soh = "{";

    soh.append("}");
	
	cout << soh << endl;


    // set the soh string
    agent->set_sohstring(soh);
}

void run_agent() {

    // Start executing the agent
    while ( agent->running() ) {
		
		handle_pycubed();

        // Sleep for a bit
        COSMOS_SLEEP(SLEEP_TIME);
    }

}


void init_pycubed() {
	
	// Create a new PyCubed device
	pycubed = new PyCubed();
	
	// Set the receive file callback
	pycubed->SetReceiveFileCallback(file_received);
	
	
	// ... TODO
}


void handle_pycubed() {
	
	// Receive messages from pycubed
	pycubed->Receive();
	
}

void file_received(const std::string &name) {
	// TODO
}



