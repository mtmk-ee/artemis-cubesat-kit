
// COSMOS headers
#include "agent/agentclass.h"
#include "support/configCosmos.h"
#include "agent/agentclass.h"
#include "support/jsonlib.h"
#include "support/jsonclass.h"


// Internal headers
#include "cubesat_defs.h"
#include "device/ADT7311.h"
#include "device/temp_sensors.h"

// Standard headers
#include <iostream>
#include <fstream>

// The interval between iterations in the run_agent() function
#define SLEEP_TIME 1

using namespace std;
using namespace cubesat;


// The agent object which allows for communication with COSMOS
Agent *agent;

// A convenient struct for storing temperature sensor information
struct TempSensor {
	ADT7311 *device; // The temperature sensor device handlers
	int dindex; // COSMOS device index
	int cindex; // COSMOS component index
	int pindex; // COSMOS piece index
	
	float temp = 0;
} temp_sensors[TEMPSENSOR_COUNT]; // Temperature sensor info, ordered according to "temp_sensors.h"




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
 * @param sensor_id The temperature sensor ID
 */
void add_sensor_piece(int sensor_id);

/**
 * @brief Initializes the sensor devices
 */
void init_sensors();

/**
 * @brief Wraps up communication with the sensor devices.
 */
void destroy_sensors();

/**
 * @brief Grabs the latest readings from the sensor devices
 */
void update_temps();

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
	agent = new Agent(CUBESAT_NODE_NAME, CUBESAT_AGENT_TEMP_NAME);
	
	// Make sure the agent initialized successfully
	if ( agent->cinfo == nullptr || !agent->running() ) {
		
		// Notify that an error occurred and exit
		printf("Failed to open [%s:%s]\n", CUBESAT_NODE_NAME, CUBESAT_AGENT_TEMP_NAME);
		exit (1);
	}
	
	// Add request callbacks
	int status;
	if ( (status = agent->add_request("gettemp", request_gettemp)) )
		exit(status);
	
	if ( (status = agent->add_request("list", request_list)) )
		exit(status);
	
	
	cout << "Successfully initialized agent" << endl;
}

void set_soh() {
	
	// Create the state of health string
	string soh = "{";
	
	for (int i = 0; i < TEMPSENSOR_COUNT; ++i) {
		// Add to the SOH string
		soh += "\"device_tsen_temp_00" + std::to_string(i) + "\", ";
		soh += "\"device_tsen_utc_00" + std::to_string(i) + "\"";
		
		// Add delimiter if necessary
		if ( i == TEMPSENSOR_COUNT - 1 )
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
		update_temps();
		
		
		// Sleep for a bit
		COSMOS_SLEEP(SLEEP_TIME);
	}
	
}

void add_sensor_piece(int sensor_id) {
	
	// Conver the temperature sensor ID number to its corresponding name
	const std::string &name = GetTempSensorName(sensor_id);
	
	// Try adding a COSMOS piece
	int pindex = json_createpiece(agent->cinfo, name, DeviceType::TSEN);
	
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
	
	temp_sensors[sensor_id].pindex = pindex;
	temp_sensors[sensor_id].cindex = cindex;
	temp_sensors[sensor_id].dindex = dindex;
	//agent->cinfo->device[agent->cinfo->pieces[pindex].cidx].bus.nvolt = 12.f;
	
}

void init_sensors() {
	
	// Add temperature sensor pieces to the COSMOS namespace
	for (int i = 0; i < TEMPSENSOR_COUNT; ++i) {
		add_sensor_piece(i);
	}
	
	// ============ Set up devices ============
	
	// Create a default configuration for the "normal" temperature sensors
	ADT7311::Configuration sensor_config;
	sensor_config.fault_queue = 0;
	sensor_config.ct_pin_polarity = 0;
	sensor_config.int_pin_polarity = 0;
	sensor_config.int_ct_mode = 0;
	sensor_config.operation_mode = 0;
	sensor_config.resolution = 0;
	
	// The temperature sensor
	ADT7311 *device;
	
	
	// Set up EPS sensor
	device = new ADT7311(); // Create the sensor device handler
	device->SetConfiguration(sensor_config); // Configure the device
	temp_sensors[TEMPSENSOR_EPS_ID].device = device;
	
	// Set up OBC sensor
	device = new ADT7311(); // Create the sensor device handler
	device->SetConfiguration(sensor_config); // Configure the device
	temp_sensors[TEMPSENSOR_OBC_ID].device = device;
	
	// Set up payload sensor
	device = new ADT7311(); // Create the sensor device handler
	device->SetConfiguration(sensor_config); // Configure the device
	temp_sensors[TEMPSENSOR_PAYLOAD_ID].device = device;
	
	// Special case for pycubed sensor. Temperatures are taken from agent_pycubed SOH
	temp_sensors[TEMPSENSOR_PYCUBED_ID].device = nullptr;
	
	// Special case for battery sensor. Temperatures are taken from agent_pycubed SOH
	temp_sensors[TEMPSENSOR_BATT_ID].device = nullptr;
	
}

void destroy_sensors() {
	
	
	// Destroy all temperature sensors
	for (int i = 0; i < TEMPSENSOR_COUNT; ++i) {
		// Call the destructor for the device handler and free memory
		delete temp_sensors[i].device;
		temp_sensors[i].device = nullptr;
	}
}

void update_regular_temp(TempSensor &sensor) {
	
	// Update readings from the device
	sensor.device->ReadState();
	
	// Retrieve the temperature reading
	float temp = sensor.device->GetTemperature();
	
	// Store device readings in COSMOS namespace
	agent->cinfo->device[sensor.cindex].tsen.utc = currentmjd(); // Timestamp of reading
	agent->cinfo->device[sensor.cindex].tsen.temp = temp; // Temperature value
}

void update_pycubed_temp() {
	static beatstruc pycubed_beat;
	
	
	// Locate agent_pycubed if not present
	if ( pycubed_beat.utc == 0. ) {
		pycubed_beat = agent->find_server(CUBESAT_NODE_NAME, CUBESAT_AGENT_PYCUBED_NAME, 5.);
		if ( pycubed_beat.utc == 0. ) {
			return;
		}
	}
	
	
	Json jresult;
	string response;
	
	
	agent->send_request(pycubed_beat, "getvalue {\"device_tsen_temp000\", \"device_cpu_temp000\"}", response);
	int status = jresult.extract_object(response);
	
	if ( status > 0 ) {
		float obc_temp = 0;
		float batt_temp = 0;
		
		// Search the response JSON for corresponding data
		for (Json::Member member : jresult.Members) {
			if ( member.value.name.find("tsen_temp") != string::npos ) {
				batt_temp = member.value.nvalue;
			}
			else if ( member.value.name.find("cpu_temp") != string::npos ) {
				obc_temp = member.value.nvalue;
			}
		}
		
		temp_sensors[TEMPSENSOR_BATT_ID].temp = batt_temp;
		temp_sensors[TEMPSENSOR_OBC_ID].temp = obc_temp;
	}
	
}

void update_temps() {
	
	// Update non-pycubed temperatures
	for (int i = 0; i < TEMPSENSOR_COUNT - 2; ++i) {
		update_regular_temp(temp_sensors[i]);
	}
	
	// Update pycubed and battery temperatures
	update_pycubed_temp();
	
}

int32_t request_gettemp(char *request, char* response, Agent *agent) {
	
	// Strip the sensor name from the request string
	char sensor_name[128];
	int status = sscanf(request, "%*s %s", sensor_name);
	
	
	// Check if the given request is invalid
	if ( status != 1 ) {
		// Print an error message
		sprintf(response, "Usage: gettemp <name>. ");
		return 1;
	}
	
	// Find the sensor ID from the name given
	int device_index = GetTempSensorID(sensor_name);
	
	// Make sure a valid sensor was selected
	if ( device_index < 0 ) {
		sprintf(response, "No sensor with the name '%s' exists.\nUse the list command to see which sensors are available", sensor_name);
		return 1;
	}
	
	// Get the latest reading stored in the COSMOS namespace
	int pindex = temp_sensors[device_index].pindex;
	float temp = agent->cinfo->device[agent->cinfo->pieces[pindex].cidx].tsen.temp;
	
	// Print the latest temperature
	sprintf(response, "%f", temp); // Print UTC as well?
	
	// Inidicate success
	return 0;
}

/**
 * @brief Lists available sensor names. Usage: "temp list"
 * @return 
 */
int32_t request_list(char *request, char* response, Agent *agent) {
	
	// Generate list of temperature sensor names
	const std::string* names = GetTempSensorNames();
	
	// Add the individual names to one string
	std::string name_list = "";
	for (int i = 0; i < TEMPSENSOR_COUNT; ++i) {
		name_list += names[i] + (i == TEMPSENSOR_COUNT - 1 ? "" : "\n");
	}
	
	// Print the names to the response string
	sprintf(response, "%s", name_list.c_str());
	
	// Inidicate success
	return 0;
}


