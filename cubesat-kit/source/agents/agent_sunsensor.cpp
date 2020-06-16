
// COSMOS headers
#include "agent/agentclass.h"
#include "support/configCosmos.h"
#include "agent/agentclass.h"
#include "support/jsonlib.h"

// Internal headers
#include "cubesat_defs.h"
#include "device/OPT3001.h"
#include "device/sun_sensors.h"

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


// A convenient struct for storing sun sensor information
struct SunSensor {
	OPT3001 *device; // The sun sensor device handler
	int dindex; // COSMOS device index
	int cindex; // COSMOS component index
	int pindex; // COSMOS piece index
	
	struct {
		float qva, qvb, qvc, qvd;
		float azimuth, elevation;
		double utc;
	} data; // Retrieved information from the sensor
} sun_sensors[SUNSENSOR_COUNT]; // Sun sensor info, ordered according to "sun_sensors.h"


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
 * @brief Adds sensor pieces to the COSMOS namespace
 * @param sensor_id The sun sensor ID
 */
void add_sensor_piece(int sensor_id);


/**
 * @brief Initializes the sensor devices
 */
void init_sensors();

/**
 * @brief Grabs the latest readings from the sensor devices
 */
void update_readings();

// Request callbacks
int32_t request_gettemp(char *request, char* response, Agent *agent);
int32_t request_list(char *request, char* response, Agent *agent);
// ===========================================







int main(int argc, char** argv) {
	
	// Initialize the agent
    init_agent();
	
	// Initialize the sun sensors
	init_sensors();
	
	// Set the state of health string for this agent
	set_soh();

	// Run the main loop for this agent
    run_agent();


    return 0;
}

void init_agent() {
    // Create the agent
    agent = new Agent(CUBESAT_NODE_NAME, CUBESAT_AGENT_SUNSENSOR_NAME);
	
	// Make sure the agent initialized successfully
    if ( agent->cinfo == nullptr || !agent->running() ) {
		
		// Notify that an error occurred and exit
        printf("Failed to open [%s:%s]\n", CUBESAT_NODE_NAME, CUBESAT_AGENT_SUNSENSOR_NAME);
        exit (1);
    }
    
    // Add request callbacks
    int status;
    if ( (status = agent->add_request("gettemp", request_gettemp)) )
        exit (status);
	
	if ( (status = agent->add_request("list", request_list)) )
        exit (status);
	
	cout << "Successfully initialized agent" << endl;
}

void set_soh() {
	
	// Create the state of health string
    string soh = "{";
	

    for (int i = 0; i < SUNSENSOR_COUNT; ++i) {
        // Add to the SOH string
        soh += "\"device_tsen_qva_00" + std::to_string(i) + "\", ";
		soh += "\"device_tsen_qvb_00" + std::to_string(i) + "\", ";
		soh += "\"device_tsen_qvc_00" + std::to_string(i) + "\", ";
		soh += "\"device_tsen_qvd_00" + std::to_string(i) + "\", ";
		soh += "\"device_tsen_azimuth_00" + std::to_string(i) + "\", ";
		soh += "\"device_tsen_elevation_00" + std::to_string(i) + "\", ";
        soh += "\"device_tsen_utc_00" + std::to_string(i) + "\"";

        // Add delimiter if necessary
        if ( i == SUNSENSOR_COUNT - 1 )
            soh += "}";
		else
			soh += ", ";
    }
	
	
	// Set the SOH string
    agent->set_sohstring(soh);
	
}

void run_agent() {
	

    // Start executing the agent
    while ( agent->running() ) {

		
        // Update sensor readings
        update_readings();
        
        
        // Sleep for a bit
        COSMOS_SLEEP(SLEEP_TIME);
    }

}

void add_sensor_piece(int sensor_id) {
	
	// Convert the sun sensor ID number to its corresponding name
	const std::string &name = GetSunSensorName(sensor_id);
	
	// Try adding a COSMOS piece
	int pindex = json_addpiece(agent->cinfo, name, (uint16_t)DeviceType::SSEN);
	
	// Check if an error occurred
	if ( pindex < 0 ) {
		// Print an error message and exit
		fprintf(agent->get_debug_fd(), "Failed to add temperature sensor '%s': %s\n", name.c_str(), cosmos_error_string(pindex).c_str());
		agent->shutdown();
		exit(1);
	}
	
	// Store the piece info for later use
	int cindex = agent->cinfo->pieces[pindex].cidx;
	int dindex = agent->cinfo->device[cindex].all.didx;
	
	sun_sensors[sensor_id].pindex = pindex;
	sun_sensors[sensor_id].cindex = cindex;
	sun_sensors[sensor_id].dindex = dindex;
}

void init_sensors() {
	
	// TODO: configure sensors and verify they are functioning
	
	// Add sun sensor pieces to the COSMOS namespace
	for (int i = 0; i < SUNSENSOR_COUNT; ++i) {
		add_sensor_piece(i);
	}
	
	// Add the +X sun sensor
	sun_sensors[SUNSENSOR_PLUSX_ID].device = new OPT3001();

	// Add the -X sun sensor
	sun_sensors[SUNSENSOR_MINUSX_ID].device = new OPT3001();
	
	// Add the +Y sun sensor
	sun_sensors[SUNSENSOR_PLUSY_ID].device = new OPT3001();
	
	// Add the -Y sun sensor
	sun_sensors[SUNSENSOR_MINUSY_ID].device = new OPT3001();
	
	// Add the +Z sun sensor
	sun_sensors[SUNSENSOR_PLUSZ_ID].device = new OPT3001();
	
	// Add the -Z sun sensor
	sun_sensors[SUNSENSOR_MINUSZ_ID].device = new OPT3001();
	
	
}

void update_reading(SunSensor &sensor) {
	
	// Update readings from the device
	sensor.device->ReadState();
	
	// Retrieve the lux reading
	float lux = sensor.device->GetLux();
	
	
	// Store device readings in COSMOS namespace
	agent->cinfo->device[sensor.cindex].ssen.utc = currentmjd(); // Timestamp of reading
	
	// TODO: store information from sun sensor
}


void update_readings() {
    cout << "Updating sun sensor log... ";
    
	// Update all the sun sensor readings
    for (int i = 0; i < SUNSENSOR_COUNT; ++i) {
		update_reading(sun_sensors[i]);
    }
    
    cout << "done." << endl;
}


int32_t request_list(char *request, char* response, Agent *agent) {
	
	// Generate list of sun sensor names
	const std::string* names = GetSunSensorNames();
	
	// Add the individual names to one string
	std::string name_list = "";
	for (int i = 0; i < SUNSENSOR_COUNT; ++i) {
		name_list += names[i] + (i == SUNSENSOR_COUNT - 1 ? "" : "\n");
	}
	
	// Print the names to the response string
	sprintf(response, "%s", name_list.c_str());
    
	// Indicate success
    return 0;
}


