
// COSMOS headers
#include "agent/agentclass.h"
#include "support/configCosmos.h"
#include "agent/agentclass.h"
#include "support/jsonlib.h"
#include "support/jsonclass.h"

// Internal headers
#include "cubesat_defs.h"
#include "device/RaspberryPi.h"
#include "device/switch.h"

// Standard headers
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <dirent.h>


// The interval between iterations in the run_agent() function
#define SLEEP_TIME 1

using namespace std;
using namespace cubesat;




// The agent object which allows for communication with COSMOS
Agent *agent;

// The raspberry pi handler

struct {
	RaspberryPi *device;
	int cindex;
	
} raspi;

struct {
	float values[10];
	int cindex;
	
} payload;


struct {
	
	struct {
		bool active;
		double utc;
		float eps_temp;
		float obc_temp;
		float raspi_temp;
		float battery_temp;
		float pycubed_temp;
	} agent_temp;
	
	struct {
		bool active;
		double utc;
		float lux_plusx;
		float lux_minusx;
		float lux_plusy;
		float lux_minusy;
		float lux_plusz;
		float lux_minusz;
	} agent_sunsensor;
	
	struct {
		bool active;
		double utc;
		struct {
			float accel[3];
			float mag[3];
			float omega[3];
		} imu;
		
		struct {
			
		} gps;
		
		struct {
			float batt_current;
			float batt_voltage;
			float sys_current;
			float sys_voltage;
		} power;
		
	} agent_pycubed;
	
	struct {
		bool active;
		double utc;
		bool switches[SWITCH_COUNT];
	} agent_switch;
	
	struct {
		bool active;
		double utc;
		bool enabled;
	} agent_heater;
	
} soh_data;


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


void handle_incoming_files();

void grab_soh_data();

int32_t request_dumpdata(char *request, char* response, Agent *agent);
int32_t request_ssh(char *request, char* response, Agent *agent);
// ===========================================





int main(int argc, char** argv) {
	
	// Initialize the agent
	init_agent();
	
	// Run the main loop for this agent
	run_agent();
	
	
	return 0;
}

void init_agent() {
	// Create the agent
	agent = new Agent(CUBESAT_NODE_NAME, CUBESAT_AGENT_RASPI_NAME);
	
	// Make sure the agent initialized successfully
	if ( agent->cinfo == nullptr || !agent->running() ) {
		printf("Failed to open [%s:%s]\n", CUBESAT_NODE_NAME, CUBESAT_AGENT_RASPI_NAME);
		exit (1);
	}
	
	int status;
	
	// Add raspberry pi
	status = json_createpiece(agent->cinfo, "raspi", DeviceType::CPU);
	if ( status < 0 ) {
		fprintf(agent->get_debug_fd(), "Failed to add raspberry pi %s\n", cosmos_error_string(status).c_str());
		agent->shutdown();
		exit(1);
	}
	
	int cindex = agent->cinfo->pieces[status].cidx;
	raspi.cindex = cindex;
	agent->cinfo->device[cindex].cpu.maxgib = 0.5f; // 512 MB RAM
	
	// Add payload
	status = json_createpiece(agent->cinfo, "payload", DeviceType::PLOAD);
	if ( status < 0 ) {
		fprintf(agent->get_debug_fd(), "Failed to add payload %s\n", cosmos_error_string(status).c_str());
		agent->shutdown();
		exit(1);
	}
	
	cindex = agent->cinfo->pieces[status].cidx;
	payload.cindex = cindex;
	
	
	
	// Create the state of health string
	string soh = "{";
	
	soh.append("}");
	
	
	
	// set the soh string
	agent->set_sohstring(soh);
	
	
	// Add request callbacks
	if ( (status = agent->add_request("dumpdata", request_dumpdata)) )
		exit(status);
	
	if ( (status = agent->add_request("ssh", request_ssh)) )
		exit(status);
}


void run_agent() {
	
	// Start executing the agent
	while ( agent->running() ) {
		
		grab_soh_data();
		
		
		// Sleep for a bit
		COSMOS_SLEEP(SLEEP_TIME);
		
		
	}
	
}

void grab_soh_data() {
	static beatstruc temp_beat, sunsensor_beat, switch_beat, heater_beat, pycubed_beat;
	const float kWaitTime = 2.;
	
	int status;
	Json jresult;
	string response;
	
	
	// Locate agents if not present
	if ( temp_beat.utc == 0. ) {
		soh_data.agent_temp.active = false;
		
		temp_beat = agent->find_agent(CUBESAT_NODE_NAME, CUBESAT_AGENT_TEMP_NAME, kWaitTime);
		if ( temp_beat.utc == 0. )
			printf("Could not find agent_temp\n");
	}
	else {
		soh_data.agent_temp.active = true;
		
		// Get temperature readings
		agent->send_request(temp_beat, "getvalue {\"device_tsen_temp_000\", \"device_tsen_temp_001\", \"device_tsen_temp_002\", \"device_tsen_temp_003\", \"device_tsen_temp004\"}", response);
		status = jresult.extract_object(response);
		
		if ( status > 0 ) {
			
			// Search the response JSON for corresponding data
			for (Json::Member member : jresult.Members) {
				if ( member.value.name.find("tsen_temp_000") != string::npos ) {
					soh_data.agent_temp.eps_temp = member.value.nvalue;
				}
				else if ( member.value.name.find("tsen_temp_001") != string::npos ) {
					soh_data.agent_temp.obc_temp = member.value.nvalue;
				}
				else if ( member.value.name.find("tsen_temp_002") != string::npos ) {
					soh_data.agent_temp.raspi_temp = member.value.nvalue;
				}
				else if ( member.value.name.find("tsen_temp_003") != string::npos ) {
					soh_data.agent_temp.battery_temp = member.value.nvalue;
				}
				else if ( member.value.name.find("tsen_temp_004") != string::npos ) {
					soh_data.agent_temp.pycubed_temp = member.value.nvalue;
				}
			}
		}
	}
	
	if ( sunsensor_beat.utc == 0. ) {
		soh_data.agent_sunsensor.active = false;
		sunsensor_beat = agent->find_server(CUBESAT_NODE_NAME, CUBESAT_AGENT_SUNSENSOR_NAME, kWaitTime);
		if ( sunsensor_beat.utc == 0. )
			printf("Could not find agent_sunsensor\n");
	}
	else {
		soh_data.agent_sunsensor.active = true;
		// TODO
	}
	
	
	if ( switch_beat.utc == 0. ) {
		soh_data.agent_switch.active = false;
		switch_beat = agent->find_server(CUBESAT_NODE_NAME, CUBESAT_AGENT_SWITCH_NAME, kWaitTime);
		if ( switch_beat.utc == 0. )
			printf("Could not find agent_switch\n");
	}
	else {
		soh_data.agent_switch.active = true;
		// Get switch info
		agent->send_request(switch_beat, "getvalue {\"device_swch_enabled_000\", \"device_swch_enabled_001\", \"device_swch_enabled_002\", \"device_swch_enabled_003\"}", response);
		status = jresult.extract_object(response);
		
		if ( status > 0 ) {
			
			// Search the response JSON for corresponding data
			for (Json::Member member : jresult.Members) {
				if ( member.value.name.find("swch_enabled_000") != string::npos ) {
					soh_data.agent_switch.switches[0] = member.value.nvalue;
				}
				else if ( member.value.name.find("swch_enabled_001") != string::npos ) {
					soh_data.agent_switch.switches[1] = member.value.nvalue;
				}
				else if ( member.value.name.find("swch_enabled_002") != string::npos ) {
					soh_data.agent_switch.switches[2] = member.value.nvalue;
				}
				else if ( member.value.name.find("swch_enabled_003") != string::npos ) {
					soh_data.agent_switch.switches[3] = member.value.nvalue;
				}
			}
		}
	}
	
	
	if ( heater_beat.utc == 0. ) {
		soh_data.agent_heater.active = false;
		heater_beat = agent->find_server(CUBESAT_NODE_NAME, CUBESAT_AGENT_HEATER_NAME, kWaitTime);
		if ( heater_beat.utc == 0. )
			printf("Could not find agent_heater\n");
	}
	else {
		soh_data.agent_heater.active = true;
		// TODO
	}
	
	
	
	
	if ( pycubed_beat.utc == 0. ) {
		soh_data.agent_pycubed.active = false;
		pycubed_beat = agent->find_server(CUBESAT_NODE_NAME, CUBESAT_AGENT_PYCUBED_NAME, kWaitTime);
		if ( pycubed_beat.utc == 0. )
			printf("Could not find agent_pycubed\n");
	}
	else {
		soh_data.agent_pycubed.active = true;
		// Get pycubed info
		agent->send_request(pycubed_beat, "getvalue {\"device_imu_mag_x_000\", \"device_imu_mag_y_000\", \"device_imu_mag_z_000\", "
										  "\"device_imu_accel_x_000\", \"device_imu_accel_y_000\", \"device_imu_accel_z_000\", "
										  "\"device_imu_omega_x_000\", \"device_imu_omega_y_000\", \"device_imu_omega_z_000\", "
										  "\"device_cpu_volt_000\", \"device_cpu_amp_000\", "
										  "\"device_batt_volt_000\", \"device_batt_amp_000\"}", response);
		status = jresult.extract_object(response);
		
		if ( status > 0 ) {
			
			// Search the response JSON for corresponding data
			for (Json::Member member : jresult.Members) {
				if ( member.value.name.find("imu_mag_x_000") != string::npos ) {
					soh_data.agent_pycubed.imu.mag[0] = member.value.nvalue;
				}
				else if ( member.value.name.find("imu_mag_y_000") != string::npos ) {
					soh_data.agent_pycubed.imu.mag[1] = member.value.nvalue;
				}
				
				else if ( member.value.name.find("imu_mag_z_000") != string::npos ) {
					soh_data.agent_pycubed.imu.mag[2] = member.value.nvalue;
				}
				
				else if ( member.value.name.find("imu_accel_x_000") != string::npos ) {
					soh_data.agent_pycubed.imu.accel[0] = member.value.nvalue;
				}
				else if ( member.value.name.find("imu_accel_y_000") != string::npos ) {
					soh_data.agent_pycubed.imu.accel[1] = member.value.nvalue;
				}
				
				else if ( member.value.name.find("imu_accel_z_000") != string::npos ) {
					soh_data.agent_pycubed.imu.accel[2] = member.value.nvalue;
				}
				
				else if ( member.value.name.find("imu_omega_x_000") != string::npos ) {
					soh_data.agent_pycubed.imu.omega[0] = member.value.nvalue;
				}
				else if ( member.value.name.find("imu_omega_y_000") != string::npos ) {
					soh_data.agent_pycubed.imu.omega[1] = member.value.nvalue;
				}
				
				else if ( member.value.name.find("imu_omega_z_000") != string::npos ) {
					soh_data.agent_pycubed.imu.omega[2] = member.value.nvalue;
				}
				
				
				else if ( member.value.name.find("cpu_volt_000") != string::npos ) {
					soh_data.agent_pycubed.power.sys_voltage = member.value.nvalue;
				}
				else if ( member.value.name.find("cpu_amp_000") != string::npos ) {
					soh_data.agent_pycubed.power.sys_current = member.value.nvalue;
				}
				
				else if ( member.value.name.find("batt_volt_000") != string::npos ) {
					soh_data.agent_pycubed.power.batt_voltage = member.value.nvalue;
				}
				else if ( member.value.name.find("batt_amp_000") != string::npos ) {
					soh_data.agent_pycubed.power.batt_current = member.value.nvalue;
				}
			}
		}
	}
	
	
	// TODO: Get sun sensor readings
}

bool get_files(const char *directory, vector<string> &out) {
	DIR *dir;
	struct dirent *ent;
	
	// Attempt to open the directory
	if ( (dir = opendir(directory)) != NULL ) {
		
		// Iterate through each file
		while ((ent = readdir(dir)) != NULL) {
			out.push_back(ent->d_name);
		}
		
		closedir(dir);
		
		return true;
	}
	else {
		fprintf(agent->get_debug_fd(), "Failed to poll contents of incoming folder %s\n", RASPI_INCOMING_FOLDER);
		
		return false;
	}
}



void handle_incoming_files() {
	
	vector<string> files;
	
	get_files(BEAGLEBONE_INCOMING_FOLDER, files);
	
	
	
	
}



int32_t request_dumpdata(char *request, char* response, Agent *agent) {
	
	stringstream ss;
	ss << "{";
	
	ss <<	"\"agent_temp\": {";
	ss <<		"\"active\": " << (soh_data.agent_temp.active ? "true" : "false") << ", ";
	ss <<		"\"temp_eps\": " << soh_data.agent_temp.eps_temp << ", ";
	ss <<		"\"temp_obc\": " << soh_data.agent_temp.obc_temp << ", ";
	ss <<		"\"temp_raspi\": " << soh_data.agent_temp.raspi_temp << ", ";
	ss <<		"\"temp_battery\": " << soh_data.agent_temp.battery_temp << ", ";
	ss <<		"\"temp_pycubed\": " << soh_data.agent_temp.pycubed_temp;
	ss <<	"}, ";
	
	ss <<	"\"agent_sunsensor\": {";
	ss <<		"\"active\": " << (soh_data.agent_sunsensor.active ? "true" : "false") << ", ";
	ss <<		"\"ss_plusx\": " << soh_data.agent_sunsensor.lux_plusx << ", ";
	ss <<		"\"ss_minusx\": " << soh_data.agent_sunsensor.lux_minusx << ", ";
	ss <<		"\"ss_plusy\": " << soh_data.agent_sunsensor.lux_plusy << ", ";
	ss <<		"\"ss_minusy\": " << soh_data.agent_sunsensor.lux_minusy << ", ";
	ss <<		"\"ss_plusz\": " << soh_data.agent_sunsensor.lux_plusz << ", ";
	ss <<		"\"ss_minusz\": " << soh_data.agent_sunsensor.lux_minusz;
	ss <<	"}, ";
	
	ss <<	"\"agent_pycubed\": {";
	ss <<		"\"active\": " << (soh_data.agent_pycubed.active ? "true" : "false") << ", ";
	ss <<		"\"imu\": {";
	ss <<			"\"accel\": [";
	ss <<				 soh_data.agent_pycubed.imu.accel[0] << ", ";
	ss <<				 soh_data.agent_pycubed.imu.accel[1] << ", ";
	ss <<				 soh_data.agent_pycubed.imu.accel[2];
	ss <<			"], ";
	ss <<			"\"mag\": [";
	ss <<				 soh_data.agent_pycubed.imu.mag[0] << ", ";
	ss <<				 soh_data.agent_pycubed.imu.mag[1] << ", ";
	ss <<				 soh_data.agent_pycubed.imu.mag[2];
	ss <<			"], ";
	ss <<			"\"gyro\": [";
	ss <<				 soh_data.agent_pycubed.imu.omega[0] << ", ";
	ss <<				 soh_data.agent_pycubed.imu.omega[1] << ", ";
	ss <<				 soh_data.agent_pycubed.imu.omega[2];
	ss <<			"]";
	ss <<		"}, ";
	ss <<		"\"power\": {";
	ss <<			"\"batt_voltage\": " << soh_data.agent_pycubed.power.batt_voltage << ", ";
	ss <<			"\"batt_current\": " << soh_data.agent_pycubed.power.batt_current << ", ";
	ss <<			"\"sys_voltage\": " << soh_data.agent_pycubed.power.sys_voltage << ", ";
	ss <<			"\"sys_current\": " << soh_data.agent_pycubed.power.sys_current;
	ss <<		"}";
	ss <<	"}, ";
	
	ss <<	"\"agent_switch\": {";
	ss <<		"\"active\": " << (soh_data.agent_switch.active ? "true" : "false") << ", ";
	ss <<		"\"enabled\": [";
	ss <<			(soh_data.agent_switch.switches[0] ? "true" : "false") << ", ";
	ss <<			(soh_data.agent_switch.switches[1] ? "true" : "false") << ", ";
	ss <<			(soh_data.agent_switch.switches[2] ? "true" : "false") << ", ";
	ss <<			(soh_data.agent_switch.switches[3] ? "true" : "false");
	ss <<		"]";
	ss <<	"}, ";
	
	ss <<	"\"agent_heater\": {";
	ss <<		"\"active\": " << (soh_data.agent_heater.active ? "true" : "false") << ", ";
	ss <<		"\"enabled\": " << (soh_data.agent_heater.enabled ? "true" : "false");
	ss <<	"},";
	
	ss <<	"\"agent_raspi\": {";
	ss <<		"\"active\": true";
	ss <<	"}";
	
	ss << "}";
	
	
	// Print the JSON to the response string
	sprintf(response, "%s", ss.str().c_str());
	
	// Inidicate success
	return 0;
}

int32_t request_ssh(char *request, char* response, Agent *agent) {
	
	char req_command[128];
	sscanf(request, "%*s %s", req_command);
	
	char command[256];
	sprintf(command, "ssh pi@raspberrypi.local %s", req_command);
	
	
	
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	
	printf("$ %s\n", command);
	printf("%s\n", result.c_str());
	
	
	// Inidicate success
	return 0;
}

