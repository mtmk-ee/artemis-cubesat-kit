
// COSMOS headers
#include "agent/agentclass.h"
#include "support/configCosmos.h"
#include "support/jsonlib.h"
#include "support/jsonclass.h"


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


// The PyCubed device handler
PyCubed *pycubed;

struct {
	struct {
		int cindex;
		int dindex;
		
	} obc;
	
	struct {
		int cindex;
		int dindex;
		
		PyCubedIMUInfo info;
	} imu;
	
	
	struct {
		int cindex;
		int dindex;
		
		PyCubedGPSInfo info;
	} gps;
	
	struct {
		int cindex;
		int dindex;
	} radio;
	
	struct {
		int cindex;
		int dindex;
		
		PyCubedBattInfo info;
	} batt;
	
	
} pycubed_info;

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


void update_namespace();




/**
 * @brief Callback function for received files from PyCubed.
 * @param name The file location
 */
void file_received(const std::string &name);

/**
 * @brief Shutdown callback function
 */
void shutdown();

// ===========================================









int main(int argc, char** argv) {
	
	// Initialize the agent
	init_agent();
	
	// Set the SOH string
	set_soh();
	
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
		exit(1);
	}
	
	int status;
	
	// Add battery (4 physically, represented as 1 for now)
	status = json_addpiece(agent->cinfo, "batt", (uint16_t)DeviceType::BATT);
	if ( status < 0 ) {
		fprintf(agent->get_debug_fd(), "Failed to add battery %s\n", cosmos_error_string(status).c_str());
		agent->shutdown();
		exit(1);
	}
	
	int cindex = agent->cinfo->pieces[status].cidx;
	pycubed_info.batt.cindex = cindex;
	agent->cinfo->device[cindex].batt.capacity = 3.5f * 4;
	agent->cinfo->device[cindex].batt.nvolt = 3.635f * 2;
	agent->cinfo->device[cindex].batt.efficiency = .85f; // Not on datasheet
	
	// Add IMU
	status = json_addpiece(agent->cinfo, "imu", (uint16_t)DeviceType::IMU);
	if ( status < 0 ) {
		fprintf(agent->get_debug_fd(), "Failed to add IMU %s\n", cosmos_error_string(status).c_str());
		agent->shutdown();
		exit(1);
	}
	cindex = agent->cinfo->pieces[status].cidx;
	pycubed_info.imu.cindex = cindex;

	// Add GPS
	status = json_addpiece(agent->cinfo, "gps", (uint16_t)DeviceType::GPS);
	if ( status < 0 ) {
		fprintf(agent->get_debug_fd(), "Failed to add GPS %s\n", cosmos_error_string(status).c_str());
		agent->shutdown();
		exit(1);
	}
	cindex = agent->cinfo->pieces[status].cidx;
	pycubed_info.gps.cindex = cindex;
	
	
	
	// Add PyCubed OBC
	status = json_addpiece(agent->cinfo, "pycubedobc", (uint16_t)DeviceType::CPU);
	if ( status < 0 ) {
		fprintf(agent->get_debug_fd(), "Failed to add PyCubed OBC %s\n", cosmos_error_string(status).c_str());
		agent->shutdown();
		exit(1);
	}
	cindex = agent->cinfo->pieces[status].cidx;
	
	pycubed_info.obc.cindex = cindex;
	pycubed_info.obc.dindex = agent->cinfo->device[cindex].all.didx;
	
	
	json_dump_node(agent->cinfo);
	
	
	// Add request callbacks (TODO)
	
	
	cout << "Successfully initialized agent" << endl;
}

void set_soh() {
	// Create the state of health string
	string soh = "{";
	
	// Add CPU info
	soh += "\"device_cpu_temp_00" + std::to_string(pycubed_info.obc.dindex) + "\",";
	
	// Add battery info
	soh += "\"device_batt_temp_000\",";
	soh += "\"device_batt_amp_000\",";
	soh += "\"device_batt_volt_000\",";
	soh += "\"device_batt_power_000\",";
	soh += "\"device_batt_charge_000\",";
	soh += "\"device_batt_percentage_000\",";
	soh += "\"device_batt_time_remaining_000\",";
	
	// Add IMU info
	soh += "\"device_imu_utc_000\",";
	soh += "\"device_imu_mag_000\",";
	soh += "\"device_imu_bdot_000\",";
	soh += "\"device_imu_accel_000\",";
	soh += "\"device_imu_alpha_000\",";
	soh += "\"device_imu_euler_000\",";
	soh += "\"device_imu_omega_000\",";
	soh += "\"device_imu_temp_000\",";
	
	// Add GPS info
	soh += "\"device_gps_utc_000\",";
	soh += "\"device_gps_geocs_000\",";
	soh += "\"device_gps_geocv_000\",";
	soh += "\"device_gps_sats_used_000\",";
	soh += "\"device_gps_sats_visible_000\",";
	soh += "\"device_gps_time_status_000\",";
	soh += "\"device_gps_position_type_000\",";
	soh += "\"device_gps_solution_status_000\",";
	
	soh += "}";
	
	
	
	// set the soh string
	agent->set_sohstring(soh);
}

void run_agent() {
	
	// Start executing the agent
	while ( agent->running() ) {
		
		handle_pycubed();
		
		update_namespace();
		
		// Sleep for a bit
		COSMOS_SLEEP(SLEEP_TIME);
	}
}


void init_pycubed() {
	
	// Create a new PyCubed device
	pycubed = new PyCubed();
	
	// Set the callback functions
	pycubed->SetShutdownCallback(shutdown);
	pycubed->SetReceiveFileCallback(file_received);
	
	// Notify the PyCubed that startup was successful
	pycubed->StartupConfirmation();
}


void handle_pycubed() {
	
	// Receive messages from pycubed
	pycubed->ReceiveMessages();
	
	// Update information
	pycubed_info.imu.info = pycubed->GetIMUInfo();
	pycubed_info.gps.info = pycubed->GetGPSInfo();
	pycubed_info.batt.info = pycubed->GetBattInfo();
}

void update_namespace() {
	int cindex;
	
	// CPU
	cindex = pycubed_info.obc.cindex;
	agent->cinfo->device[cindex].cpu.utc = currentmjd();
	agent->cinfo->device[cindex].cpu.temp = pycubed_info.imu.info.temp;
	//agent->cinfo->device[cindex].cpu.amp = i3v3_draw + i5v_draw;
	//agent->cinfo->device[cindex].cpu.power = i3v3_draw * 3.3 + i5v_draw * 5;
	//agent->cinfo->device[cindex].cpu.volt = agent->cinfo->device[eps_cidx].cpu.power / agent->cinfo->device[eps_cidx].cpu.amp;
	//agent->cinfo->device[cindex].cpu.energy = tpower.bus[4].energy + tpower.bus[5].energy;
	
	
	// Battery
	cindex = pycubed_info.batt.cindex;
	agent->cinfo->device[cindex].batt.utc = currentmjd();
	agent->cinfo->device[cindex].batt.temp = pycubed_info.batt.info.temp;
	agent->cinfo->device[cindex].batt.amp = pycubed_info.batt.info.amp;
	agent->cinfo->device[cindex].batt.volt = pycubed_info.batt.info.volt;
	agent->cinfo->device[cindex].batt.charge = pycubed_info.batt.info.charge;
	agent->cinfo->device[cindex].batt.percentage = pycubed_info.batt.info.percentage;
	agent->cinfo->device[cindex].batt.time_remaining = pycubed_info.batt.info.time_remaining;
	
	// IMU
	cindex = pycubed_info.imu.cindex;
	agent->cinfo->device[cindex].imu.utc = currentmjd();
	agent->cinfo->device[cindex].imu.temp = pycubed_info.imu.info.temp;
	agent->cinfo->device[cindex].imu.mag = pycubed_info.imu.info.mag;
	agent->cinfo->device[cindex].imu.bdot = pycubed_info.imu.info.bdot;
	agent->cinfo->device[cindex].imu.accel = pycubed_info.imu.info.accel;
	agent->cinfo->device[cindex].imu.alpha = pycubed_info.imu.info.alpha;
	agent->cinfo->device[cindex].imu.euler = pycubed_info.imu.info.euler;
	agent->cinfo->device[cindex].imu.omega = pycubed_info.imu.info.omega;
	agent->cinfo->device[cindex].imu.theta = pycubed_info.imu.info.theta;
	
	
	// GPS
	agent->cinfo->device[cindex].gps.utc = currentmjd();
	agent->cinfo->device[cindex].gps.geocs = pycubed_info.gps.info.geocs;
	agent->cinfo->device[cindex].gps.geocv = pycubed_info.gps.info.geocv;
	agent->cinfo->device[cindex].gps.sats_used = pycubed_info.gps.info.sats_used;
	agent->cinfo->device[cindex].gps.sats_visible = pycubed_info.gps.info.sats_visible;
	agent->cinfo->device[cindex].gps.time_status = pycubed_info.gps.info.time_status;
	agent->cinfo->device[cindex].gps.position_type = pycubed_info.gps.info.position_type;
	agent->cinfo->device[cindex].gps.solution_status = pycubed_info.gps.info.solution_status;
	
	// Node
	agent->cinfo->node.utc = currentmjd();
	//agent->cinfo->node.powuse = tpower.power_flow_use;
	//agent->cinfo->node.powgen = tpower.power_flow_gen;
	
	
}

void file_received(const std::string &name) {
	// TODO
}

void shutdown() {
	// 1. Shut down agents (not agent_switch)
	
	// 2. Turn off hardware components (agent_switch)
	
	// 3. Give PyCubed permission to take over
	pycubed->Handoff();
	agent->shutdown();
	exit(0);
}


