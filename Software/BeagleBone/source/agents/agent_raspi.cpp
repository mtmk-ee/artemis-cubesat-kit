
#include "utility/SimpleAgent.h"
#include "device/RaspberryPi.h"
#include "device/switch.h"

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <dirent.h>


// The interval between iterations in the run_agent() function
#define SLEEP_TIME 1
#define RASPI_HOST "raspberrypi.local"

using namespace std;
using namespace cubesat;

// The agent object which allows for communication with COSMOS
SimpleAgent *agent;
CPU *raspi;
Camera *camera;
CustomDevice *pycubed, *tempsensors, *sunsensors, *switches, *heater;
bool perform_shutdown = false;


Timer up_time_timer;

// =========== Function Prototypes ===========
//! Attempts to connect to the Raspberry Pi by pinging it
void ConnectRaspi();
//! Updates the Raspberry Pi
void UpdateRaspi();
//! Reads and stores SOH data from other agents
void GrabSOHData();
//! Calls a system command on the Raspberry Pi
void SystemCall(const std::string &command, std::string &output);

string Request_GetData();
string Request_SSH(vector<string> args);
string Request_Ping();
string Request_Shutdown();
// ===========================================


int main(int argc, char** argv) {
	
	// Create the agent
	agent = new SimpleAgent(CUBESAT_AGENT_RASPI_NAME);
	agent->SetLoopPeriod(SLEEP_TIME);
	
	// Add the camera
	camera = agent->NewDevice<Camera>("camera");
	camera->Post(camera->utc = 0);
	camera->Post(camera->enabled = false);
	camera->Post(camera->pixel_width = 1280);
	camera->Post(camera->pixel_height = 700);
	
	// Add the Raspberry Pi
	raspi = agent->NewDevice<CPU>("raspi");
	raspi->Post(raspi->utc = Time::Now());
	raspi->Post(raspi->temperature = 273.15);
	raspi->Post(raspi->load = 0);
	raspi->Post(raspi->memory_usage = 0);
	raspi->Post(raspi->max_memory = 0.5);
	raspi->Post(raspi->boot_count = 0);
	raspi->Post(raspi->up_time = 0);
	raspi->SetCustomProperty<bool>("connected", false);
	
	// Add custom devices to hold SOH data
	tempsensors = agent->NewDevice<CustomDevice>("agent_temp");
	sunsensors = agent->NewDevice<CustomDevice>("agent_sunsensor");
	heater = agent->NewDevice<CustomDevice>("agent_heater");
	pycubed = agent->NewDevice<CustomDevice>("agent_pycubed");
	switches = agent->NewDevice<CustomDevice>("agent_switch");
	
	agent->Finalize();
	
	// Add requests
	agent->AddRequest({"get_data", "agent_data"}, Request_GetData, "Prints data collected from other agents");
	agent->AddRequest({"ssh", "command"}, Request_SSH, "Runs a command on the Raspberry Pi");
	agent->AddRequest({"ping", "is_up"}, Request_Ping, "Checks if the Raspberry Pi is up");
	agent->AddRequest({"shutdown_raspi", "end"}, Request_Shutdown, "Attempts to shut down the Raspberry Pi");
	
	agent->DebugPrint();
	
	
	// Run the main loop for this agent
	while ( agent->StartLoop() ) {
		ConnectRaspi();
		UpdateRaspi();
		GrabSOHData();
	}
	
	delete agent;
	
	
	return 0;
}

void ConnectRaspi() {
	
	const char *ping_cmd = "ping -c1 -W1 " RASPI_HOST " > nul  && echo 'UP' || echo 'DOWN'";
	std::string response;
	const int kNumAttempts = 3;
	
	raspi->utc = Time::Now();
	
	// Attempt to connect some number of times
	for (int i = 0; i < kNumAttempts; ++i) {
		// Run the ping command to check if the raspi is up
		SystemCall(ping_cmd, response);
		
		// Check the output of the ping command
		if ( response.find("UP") != string::npos ) {
			printf("RaspberryPi is UP\n");
			
			// Set the connection time
			if ( !raspi->GetCustomProperty<bool>("connected") ) {
				up_time_timer.Start();
			}
			
			raspi->SetCustomProperty<bool>("connected", true);
			raspi->up_time = (int)up_time_timer.Seconds();
			
			return;
		}
		else
			printf("Cannot reach Raspbery Pi. Attempting to reconnect...\n");
	}
	
	raspi->SetCustomProperty<bool>("connected", false);
	raspi->up_time = 0;
	
	printf("Failed to connect to Raspberry Pi. Will attempt on next cycle.\n");
}

void UpdateRaspi() {
	if ( !raspi->GetCustomProperty<bool>("connected") ) {
		
		if ( perform_shutdown ) {
			printf("Cannot shut down the Raspberry Pi: it is not connected\n");
			perform_shutdown = false;
		}
		
		return;
	}
	
	if ( perform_shutdown ) {
		
		printf("Attempting to shut down Raspberry Pi\n");
		
		// Create the command string
		string command = "ssh pi@raspberrypi.local sudo shutdown now";
		
		
		// Call the command
		string output;
		SystemCall(command, output);
		
		perform_shutdown = false;
	}
}


void GrabSOHData() {
	static RemoteAgent agent_temp = agent->FindAgent(CUBESAT_AGENT_TEMP_NAME);
	static RemoteAgent agent_sunsensor = agent->FindAgent(CUBESAT_AGENT_SUNSENSOR_NAME);
	static RemoteAgent agent_heater = agent->FindAgent(CUBESAT_AGENT_HEATER_NAME);
	static RemoteAgent agent_switch = agent->FindAgent(CUBESAT_AGENT_SWITCH_NAME);
	static RemoteAgent agent_pycubed = agent->FindAgent(CUBESAT_AGENT_PYCUBED_NAME);
	
	// Get values from agent_temp
	if ( agent_temp.Connect() ) {
		auto values = agent_temp.GetCOSMOSValues({"device_tsen_temp_000", "device_tsen_utc_000",
											"device_tsen_temp_001", "device_tsen_utc_001",
											"device_tsen_temp_002", "device_tsen_utc_002",
											"device_tsen_temp_003", "device_tsen_utc_003",
											"device_tsen_temp_004", "device_tsen_utc_004"});
		if ( !values.empty() ) {
			tempsensors->SetCustomProperty<bool>("active", true);
			
			tempsensors->SetCustomProperty<float>("temp_eps", values["device_tsen_temp_000"].nvalue);
			tempsensors->SetCustomProperty<float>("temp_obc", values["device_tsen_temp_001"].nvalue);
			tempsensors->SetCustomProperty<float>("temp_raspi", values["device_tsen_temp_002"].nvalue);
			tempsensors->SetCustomProperty<float>("temp_battery", values["device_tsen_temp_003"].nvalue);
			tempsensors->SetCustomProperty<float>("temp_pycubed", values["device_tsen_temp_004"].nvalue);
			
			tempsensors->SetCustomProperty<double>("utc_eps", values["device_tsen_utc_000"].nvalue);
			tempsensors->SetCustomProperty<double>("utc_obc", values["device_tsen_utc_001"].nvalue);
			tempsensors->SetCustomProperty<double>("utc_raspi", values["device_tsen_utc_002"].nvalue);
			tempsensors->SetCustomProperty<double>("utc_battery", values["device_tsen_utc_003"].nvalue);
			tempsensors->SetCustomProperty<double>("utc_pycubed", values["device_tsen_utc_004"].nvalue);
			
			raspi->temperature = values["device_tsen_temp_002"].nvalue;
		}
		else
			tempsensors->SetCustomProperty<bool>("active", false);
	}
	else
		tempsensors->SetCustomProperty<bool>("active", false);
	
	// Get values from agent_sunsensor
	if ( agent_sunsensor.Connect() ) {
		auto values = agent_sunsensor.GetCOSMOSValues({"device_ssen_temp_000", "device_ssen_utc_000",
												 "device_ssen_temp_001", "device_ssen_utc_001",
												 "device_ssen_temp_002", "device_ssen_utc_002",
												 "device_ssen_temp_003", "device_ssen_utc_003",
												 "device_ssen_temp_004", "device_ssen_utc_004",
												 "device_ssen_temp_005", "device_ssen_utc_005"});
		if ( !values.empty() ) {
			sunsensors->SetCustomProperty<bool>("active", true);
			
			sunsensors->SetCustomProperty<float>("lux_plusx", values["device_ssen_temp_000"].nvalue);
			sunsensors->SetCustomProperty<float>("lux_minusx", values["device_ssen_temp_001"].nvalue);
			sunsensors->SetCustomProperty<float>("lux_plusy", values["device_ssen_temp_002"].nvalue);
			sunsensors->SetCustomProperty<float>("lux_minusy", values["device_ssen_temp_003"].nvalue);
			sunsensors->SetCustomProperty<float>("lux_plusy", values["device_ssen_temp_004"].nvalue);
			sunsensors->SetCustomProperty<float>("lux_minusy", values["device_ssen_temp_005"].nvalue);
			
			sunsensors->SetCustomProperty<double>("utc_plusx", values["device_ssen_utc_000"].nvalue);
			sunsensors->SetCustomProperty<double>("utc_minusx", values["device_ssen_utc_001"].nvalue);
			sunsensors->SetCustomProperty<double>("utc_plusy", values["device_ssen_utc_002"].nvalue);
			sunsensors->SetCustomProperty<double>("utc_minusy", values["device_ssen_utc_003"].nvalue);
			sunsensors->SetCustomProperty<double>("utc_plusz", values["device_ssen_utc_004"].nvalue);
			sunsensors->SetCustomProperty<double>("utc_minusz", values["device_ssen_utc_005"].nvalue);
		}
		else
			sunsensors->SetCustomProperty<bool>("active", false);
	}
	else
		sunsensors->SetCustomProperty<bool>("active", false);
	
	// Get values from agent_heater
	if ( agent_heater.Connect() ) {
		auto values = agent_heater.GetCOSMOSValues({"device_htr_volt_000", "device_htr_utc_000"});
		
		if ( !values.empty() ) {
			heater->SetCustomProperty<bool>("active", true);
			
			heater->SetCustomProperty<bool>("enabled", values["device_htr_volt_000"].nvalue != 0.0);
			heater->SetCustomProperty<double>("utc", values["device_htr_utc_000"].nvalue);
		}
	}
	
	// Get values from agent_switch
	if ( agent_switch.Connect() ) {
		auto values = agent_switch.GetCOSMOSValues({"device_swch_volt_000", "device_swch_utc_000",
											  "device_swch_volt_001", "device_swch_utc_001",
											  "device_swch_volt_002", "device_swch_utc_002"});
		if ( !values.empty() ) {
			switches->SetCustomProperty<bool>("active", true);
			
			switches->SetCustomProperty<bool>("sw_temp", values["device_swch_volt_000"].nvalue != 0.0);
			switches->SetCustomProperty<bool>("sw_sunsensor", values["device_swch_volt_001"].nvalue != 0.0);
			switches->SetCustomProperty<bool>("sw_heater", values["device_swch_volt_002"].nvalue != 0.0);
			
			switches->SetCustomProperty<double>("utc_temp", values["device_swch_utc_000"].nvalue);
			switches->SetCustomProperty<double>("utc_sunsensor", values["device_swch_utc_001"].nvalue);
			switches->SetCustomProperty<double>("utc_heater", values["device_swch_utc_002"].nvalue);
		}
		else
			switches->SetCustomProperty<bool>("active", false);
	}
	else
		switches->SetCustomProperty<bool>("active", false);
	
	// Get values from agent_pycubed
	if ( agent_pycubed.Connect() ) {
		auto values = agent_switch.GetCOSMOSValues({"device_imu_mag_x_000", "device_imu_mag_y_000", "device_imu_mag_z_000",
											  "device_imu_accel_x_000", "device_imu_accel_y_000", "device_imu_accel_z_000",
											  "device_imu_omega_x_000", "device_imu_omega_y_000", "device_imu_omega_z_000", "device_imu_utc_000",
											  "device_cpu_volt_000", "device_cpu_amp_000", "device_cpu_utc_000",
											  "device_batt_volt_000", "device_batt_amp_000", "device_batt_utc_000",
											  "device_gps_utc_000", "device_gps_geods_000", "device_gps_geocv_000", "device_gps_sats_used_000"});
		
		if ( !values.empty() ) {
			pycubed->SetCustomProperty<bool>("active", true);
			
			pycubed->SetCustomProperty<double>("imu_utc", values["device_imu_utc_000"].nvalue);
			pycubed->SetCustomProperty<float>("imu_mag_x", values["device_imu_mag_x_000"].nvalue);
			pycubed->SetCustomProperty<float>("imu_mag_y", values["device_imu_mag_y_000"].nvalue);
			pycubed->SetCustomProperty<float>("imu_mag_z", values["device_imu_mag_z_000"].nvalue);
			pycubed->SetCustomProperty<float>("imu_accel_x", values["device_imu_accel_x_000"].nvalue);
			pycubed->SetCustomProperty<float>("imu_accel_y", values["device_imu_accel_y_000"].nvalue);
			pycubed->SetCustomProperty<float>("imu_accel_z", values["device_imu_accel_z_000"].nvalue);
			pycubed->SetCustomProperty<float>("imu_gyro_x", values["device_imu_omega_000"].nvalue);
			pycubed->SetCustomProperty<float>("imu_gyro_y", values["device_imu_omega_y_000"].nvalue);
			pycubed->SetCustomProperty<float>("imu_gyro_z", values["device_imu_omega_z_000"].nvalue);
			
			pycubed->SetCustomProperty<float>("power_utc", values["device_cpu_utc_000"].nvalue);
			pycubed->SetCustomProperty<float>("sys_voltage", values["device_cpu_volt_000"].nvalue);
			pycubed->SetCustomProperty<float>("sys_current", values["device_cpu_amp_000"].nvalue);
			pycubed->SetCustomProperty<float>("batt_voltage", values["device_batt_volt_000"].nvalue);
			pycubed->SetCustomProperty<float>("batt_current", values["device_batt_amp_000"].nvalue);
			
			pycubed->SetCustomProperty<float>("gps_utc", values["device_gps_utc_000"].nvalue);
			pycubed->SetCustomProperty<float>("gps_latitude", values["device_gps_geods_lat_000"].nvalue);
			pycubed->SetCustomProperty<float>("gps_longitude", values["device_gps_geods_lon_000"].nvalue);
			pycubed->SetCustomProperty<float>("gps_altitude", values["device_gps_geods_h_000"].nvalue);
			pycubed->SetCustomProperty<float>("gps_velocity_x", values["device_gps_geocv_x_000"].nvalue);
			pycubed->SetCustomProperty<float>("gps_velocity_y", values["device_gps_geocv_y_000"].nvalue);
			pycubed->SetCustomProperty<float>("gps_velocity_z", values["device_gps_geocv_z_000"].nvalue);
			pycubed->SetCustomProperty<int>("gps_satellites", (int)values["device_gps_sats_used_000"].nvalue);
			
		}
		else
			pycubed->SetCustomProperty<bool>("active", false);
	}
	else
		pycubed->SetCustomProperty<bool>("active", false);
}


void SystemCall(const std::string &command, std::string &output) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
	if ( !pipe ) {
		throw std::runtime_error("popen() failed!");
	}
	
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	
	output = result;
}


string Request_GetData() {
	
	stringstream ss;
	ss << "{";
	
	ss <<	"\"agent_temp\": {";
	ss <<		"\"active\": " << (tempsensors->GetCustomProperty<bool>("active") ? "true" : "false") << ", ";
	ss <<		"\"temp_eps\": " << tempsensors->GetCustomProperty<float>("temp_eps") << ", ";
	ss <<		"\"temp_obc\": " << tempsensors->GetCustomProperty<float>("temp_obc") << ", ";
	ss <<		"\"temp_raspi\": " << tempsensors->GetCustomProperty<float>("temp_raspi") << ", ";
	ss <<		"\"temp_battery\": " << tempsensors->GetCustomProperty<float>("temp_battery") << ", ";
	ss <<		"\"temp_pycubed\": " << tempsensors->GetCustomProperty<float>("temp_pycubed");
	ss <<	"}, ";
	
	ss <<	"\"agent_sun\": {";
	ss <<		"\"active\": " << (sunsensors->GetCustomProperty<bool>("active") ? "true" : "false") << ", ";
	ss <<		"\"ss_plusx\": " << tempsensors->GetCustomProperty<float>("lux_plusx") << ", ";
	ss <<		"\"ss_minusx\": " << tempsensors->GetCustomProperty<float>("lux_minusx") << ", ";
	ss <<		"\"ss_plusy\": " << tempsensors->GetCustomProperty<float>("lux_plusy") << ", ";
	ss <<		"\"ss_minusy\": " << tempsensors->GetCustomProperty<float>("lux_minusy") << ", ";
	ss <<		"\"ss_plusz\": " << tempsensors->GetCustomProperty<float>("lux_plusz") << ", ";
	ss <<		"\"ss_minusz\": " << tempsensors->GetCustomProperty<float>("lux_minusz");
	ss <<	"}, ";
	
	ss <<	"\"agent_pycubed\": {";
	ss <<		"\"active\": " << (pycubed->GetCustomProperty<bool>("active") ? "true" : "false") << ", ";
	ss <<		"\"imu\": {";
	ss <<			"\"accel\": [";
	ss <<				 pycubed->GetCustomProperty<float>("imu_accel_x") << ", ";
	ss <<				 pycubed->GetCustomProperty<float>("imu_accel_y") << ", ";
	ss <<				 pycubed->GetCustomProperty<float>("imu_accel_z");
	ss <<			"], ";
	ss <<			"\"mag\": [";
	ss <<				 pycubed->GetCustomProperty<float>("imu_mag_x") << ", ";
	ss <<				 pycubed->GetCustomProperty<float>("imu_mag_y") << ", ";
	ss <<				 pycubed->GetCustomProperty<float>("imu_mag_z");
	ss <<			"], ";
	ss <<			"\"gyro\": [";
	ss <<				 pycubed->GetCustomProperty<float>("imu_gyro_x") << ", ";
	ss <<				 pycubed->GetCustomProperty<float>("imu_gyro_y") << ", ";
	ss <<				 pycubed->GetCustomProperty<float>("imu_gyro_z");
	ss <<			"]";
	ss <<		"}, ";
	ss <<		"\"gps\": {";
	ss <<			"\"utc\": " << pycubed->GetCustomProperty<float>("gps_utc") << ", ";
	ss <<			"\"latitude\": " << pycubed->GetCustomProperty<float>("gps_latitude") << ", ";
	ss <<			"\"longitude\": " << pycubed->GetCustomProperty<float>("gps_longitude") << ", ";
	ss <<			"\"altitude\": " << pycubed->GetCustomProperty<float>("gps_altitude") << ", ";
	ss <<			"\"satellites\": " << pycubed->GetCustomProperty<int>("gps_satellites") << ", ";
	ss <<			"\"velocity\": [";
	ss <<				 pycubed->GetCustomProperty<float>("gps_velocity_x") << ", ";
	ss <<				 pycubed->GetCustomProperty<float>("gps_velocity_y") << ", ";
	ss <<				 pycubed->GetCustomProperty<float>("gps_velocity_z");
	ss <<			"]";
	ss <<		"}, ";
	ss <<		"\"power\": {";
	ss <<			"\"batt_voltage\": " << pycubed->GetCustomProperty<float>("batt_voltage") << ", ";
	ss <<			"\"batt_current\": " << pycubed->GetCustomProperty<float>("batt_current") << ", ";
	ss <<			"\"sys_voltage\": " << pycubed->GetCustomProperty<float>("sys_voltage") << ", ";
	ss <<			"\"sys_current\": " << pycubed->GetCustomProperty<float>("sys_current");
	ss <<		"}";
	ss <<	"}, ";
	
	ss <<	"\"agent_switch\": {";
	ss <<		"\"active\": " << (switches->GetCustomProperty<bool>("active") ? "true" : "false") << ", ";
	ss <<		"\"enabled\": [";
	ss <<			(switches->GetCustomProperty<bool>("sw_temp") ? "true" : "false") << ", ";
	ss <<			(switches->GetCustomProperty<bool>("sw_sunsensor") ? "true" : "false") << ", ";
	ss <<			(switches->GetCustomProperty<bool>("sw_heater") ? "true" : "false") << ", ";
	ss <<		"]";
	ss <<	"}, ";
	
	ss <<	"\"agent_heater\": {";
	ss <<		"\"active\": " << (heater->GetCustomProperty<bool>("active") ? "true" : "false") << ", ";
	ss <<		"\"enabled\": " << (heater->GetCustomProperty<bool>("enabled") ? "true" : "false");
	ss <<	"},";
	
	ss <<	"\"agent_raspi\": {";
	ss <<		"\"active\": true";
	ss <<	"}";
	
	ss << "}";
	
	// Return the JSON string
	return ss.str();
}


string Request_SSH(vector<string> args) {
	
	// Make sure we actually have arguments
	if ( args.size() == 0 )
		return "Usage: ssh command";
	
	// Create the command string
	string command = "ssh pi@raspberrypi.local";
	for (const std::string &arg : args)
		command += " " + arg;
	
	
	// Call the command
	string output;
	printf("$ %s\n", command.c_str());
	SystemCall(command, output);
	printf("%s\n", output.c_str());
	
	// Return the output of the shell command
	return output;
}

string Request_Ping() {
	const char *ping_cmd = "ping -c1 -W1 " RASPI_HOST " > nul  && echo 'UP' || echo 'DOWN'";
	std::string response;
	const int kNumAttempts = 3;
	
	// Attempt to connect some number of times
	for (int i = 0; i < kNumAttempts; ++i) {
		// Run the ping command to check if the raspi is up
		SystemCall(ping_cmd, response);
		
		// Check the output of the ping command
		if ( response.find("UP") != string::npos ) {
			return "UP";
		}
	}
	return "DOWN";
}
string Request_Shutdown() {
	perform_shutdown = true;
	return "OK";
}

