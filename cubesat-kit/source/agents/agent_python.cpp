
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

// Third party headers
#include <thirdparty/pybind11/embed.h>


// The interval between iterations in the run_agent() function
#define SLEEP_TIME 1

using namespace std;
using namespace cubesat;
namespace py = pybind11;


// The agent object which allows for communication with COSMOS
Agent *agent;

string script_folder = "/home/osboxes/cosmos/source/projects/cubesat-kit/scripts/";

std::vector<string> load_queue; // List of names of scripts that need loading
std::vector<string> reload_queue; // List of names of scripts that need reloading

/**
 * @brief Stores information about a script module
 */
struct Script {
	py::module module; // The python module referring to the script
	py::function update_callback; // The python function to call for updates
	float update_interval = -1; // The interval to wait between update calls (-1 for no update)
	
	/**
	 * @brief Reloads this script
	 */
	void Reload() {
		module.reload();
	}
};

std::unordered_map<string, Script> modules;




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
 * @brief Loads a python script with the given name
 * @param name The name of the script
 */
void load_script(const std::string &name);
/**
 * @brief Reloads an already-loaded python script
 * @param name The name of the script
 */
void reload_script(const std::string &name);
/**
 * @brief Checks if a python script is already loaded
 * @param name The name of the script
 * @return True if loaded, false otherwise
 */
bool module_loaded(const std::string &name);

// Request callbacks
int32_t request_load(char *request, char* response, Agent *agent);
int32_t request_reload(char *request, char* response, Agent *agent);
// ===========================================


// =================== Stuff for Embedding Python ===================

/**
 * @brief Storage for temperature sensor information
 */
struct _Cosmos_TempSensor {
	string sensor_name;
	float temperature;
	
};

/**
 * @brief Storage for sun sensor information
 */
struct _Cosmos_SunSensor {
	string sensor_name;
	//...
};

/**
 * @brief Called from Python. Sets the update callback for a script
 * @param script_name The name of the script
 * @param func The callback function
 * @param interval The update interval (sec)
 * @return True if successful
 */
bool _cosmos_script_setupdater(const string &script_name, py::object &func, float interval);

/**
 * @brief Called from Python. Retrieves the latest temperature readings from a temperature sensor.
 * @param temp_sensor_name The name of the temperature sensor
 * @return A struct containing the temperature sensor info
 */
_Cosmos_TempSensor _cosmos_tempsensor_getlatest(const string &temp_sensor_name);

/**
 * @brief Called from Python. Retrieves the latest sun sensor readings.
 * @param sun_sensor_name The name of the sun sensor
 * @return A struct containing the sun sensor info
 */
_Cosmos_SunSensor _cosmos_sunsensor_getlatest(const string &sun_sensor_name);


// Code for embedding the "cosmos" module.
// This part exposes the functionality we want to Python scripts.
PYBIND11_EMBEDDED_MODULE(cosmos, m) {
	
	// Define the "TempSensor" class
	py::class_<_Cosmos_TempSensor>(m, "TempSensor")
		.def(py::init<>())
		.def_readwrite("name", &_Cosmos_TempSensor::sensor_name)
		.def_readwrite("temp", &_Cosmos_TempSensor::temperature);
	
	// Define the "Sun Sensor" class
	py::class_<_Cosmos_SunSensor>(m, "SunSensor")
		.def(py::init<>())
		.def_readwrite("name", &_Cosmos_SunSensor::sensor_name);
	
	// Add the "set update callback" function
	m.def("SetUpdateCallback", &_cosmos_script_setupdater);
	
	// Add the temperature sensor submodule
	m.def_submodule("tempsensor")
		// Add the temp. info getter function
		.def("GetLatest", &_cosmos_tempsensor_getlatest);
	
	// Sun Sensor Submodule
	m.def_submodule("sunsensor")
		// Add the sun sensor getter function
		.def("GetLatest", &_cosmos_sunsensor_getlatest);
}

// Create the Python interpreter.
// This line must be added AFTER all module definitions!
py::scoped_interpreter guard {};

// ==================================================================

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
    agent = new Agent(CUBESAT_NODE_NAME, CUBESAT_AGENT_PYTHON_NAME);

	// Make sure the agent initialized successfully
    if ( agent->cinfo == nullptr || !agent->running() ) {
		
		// Notify that an error occurred and exit
        printf("Failed to open [%s:%s]\n", CUBESAT_NODE_NAME, CUBESAT_AGENT_PYTHON_NAME);
        exit (1);
    }
    
    // Add request callbacks
    int status;
    
	if ( (status = agent->add_request("load", request_load)) )
        exit (status);
    if ( (status = agent->add_request("reload", request_reload)) )
        exit (status);
	
	
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
	
	// Change the working directory for the python scripts
	py::exec("import sys\nsys.path.append('" + script_folder + "')");
	
	
    // Start executing the agent
    while ( agent->running() ) {
		
		// Load scripts
		for (string &name : load_queue) {
			load_script(name);
		}
		load_queue.clear();
		
		// Reload scripts
		for (string &name : reload_queue) {
			reload_script(name);
		}
		reload_queue.clear();
		
		// Handle update callbacks
		for (auto &pair : modules) {
			
			// Check if the update interval is set
			if ( pair.second.update_interval > 0 ) {
				// Invoke the update callback function
				pair.second.update_callback();
			}
		}
	

        // Sleep for a bit
        COSMOS_SLEEP(SLEEP_TIME);
    }

}

int32_t request_load(char *request, char* response, Agent *agent) {
	
	// Strip out the script name from the request string
	char name[128];
	
	// Check if the module is already loaded
	if ( !module_loaded(name) ) {
		// TODO: Make sure the script exists before adding it to the queue
		
		// Add the script to the load queue
		load_queue.push_back(name);
		
		// Print success response
		sprintf(response, "Loading module '%s'.", name);
		
		return 0;
	}
	else {
		// If the script is already loaded, print an error message
		sprintf(response, "Module '%s' is already loaded.", name);
		return 1;
	}
}


int32_t request_reload(char *request, char* response, Agent *agent) {
	// Strip out the script name from the request string
	char name[128];
	sscanf(request, "%*s %s", name);
	
	// Check if the script is already loaded
	if ( module_loaded(name) ) {
		// Add the script to the reload queue
		reload_queue.push_back(name);
		
		// Print success response
		sprintf(response, "Reloading module '%s'.", name);
		return 0;
	}
	else {
		// If the script is not already loaded, print an error message
		sprintf(response, "Module '%s' is not already loaded.", name);
		return 1;
	}
}

void reload_script(const std::string &name) {
	if ( module_loaded(name) ) {
		// Call the reload function on the module
		modules[name].Reload();
	}
}

void load_script(const std::string &name) {
	// Make sure the module isn't already loaded
	if ( module_loaded(name) )
		return; // TODO: signal error
	
	// Import the Python module
	py::module module = py::module::import(name.c_str());
	
	// Create a Script object referring to this module
	Script script;
	script.module = module;
	
	// Store the Script object by name
	modules[name] = script;
}
bool module_loaded(const std::string &name) {
	// Search the unordered_map for scripts with the given name
	return modules.find(name) != modules.end();
}

// =============================================================================================

bool _cosmos_script_setupdater(const string &script_name, py::object &func, float interval) {
	
	// Signal an error if the module isn't even loaded
	if ( !module_loaded(script_name) )
		return false;
	
	// Store the callback information
	modules[script_name].update_callback = func;
	modules[script_name].update_interval = interval;
	
	// Indicate success condition
	return true;
}

_Cosmos_TempSensor _cosmos_tempsensor_getlatest(const string &temp_sensor_name) {
	// TODO
	
}
_Cosmos_SunSensor _cosmos_sunsensor_getlatest(const string &sun_sensor_name) {
	// TODO
}
