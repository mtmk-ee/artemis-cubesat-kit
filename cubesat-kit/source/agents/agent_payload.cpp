
// COSMOS headers
#include "agent/agentclass.h"
#include "support/configCosmos.h"
#include "agent/agentclass.h"
#include "support/jsonlib.h"

// Internal headers
#include "cubesat_defs.h"

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
// ===========================================





int main(int argc, char** argv) {

	// Initialize the agent
    init_agent();
	
	// Set the state of health string for this agent
	set_soh();

	// Run the main loop for this agent
    run_agent();


    return 0;
}

void init_agent() {
    // Create the agent
    agent = new Agent(CUBESAT_NODE_NAME, CUBESAT_AGENT_PAYLOAD_NAME);

	// Make sure the agent initialized successfully
    if ( agent->cinfo == nullptr || !agent->running() ) {
        printf("Failed to open [%s:%s]\n", CUBESAT_NODE_NAME, CUBESAT_AGENT_PAYLOAD_NAME);
        exit (1);
    }
	
	cout << "Successfully initialized agent" << endl;
}

void set_soh() {
	// Create the state of health string
    string soh = "{";

    soh.append("}");
	


    // set the soh string
    agent->set_sohstring(soh);
}


void run_agent() {

    // Start executing the agent
    while ( agent->running() ) {
		// TODO

        // Sleep for a bit
        COSMOS_SLEEP(SLEEP_TIME);
    }

}



