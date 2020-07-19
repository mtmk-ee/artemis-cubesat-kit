
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
Device *raspi, *camera;
Device *pycubed, *tempsensors, *sunsensors, *switches, *heater;

// =========== Function Prototypes ===========
//! Attempts to connect to the Raspberry Pi by pinging it
void ConnectRaspi();
//! Reads and stores SOH data from other agents
void GrabSOHData();
//! Calls a system command on the Raspberry Pi
void SystemCall(const std::string &command, std::string &output);
//! Returns the duration for which the Raspberry Pi has been connected
size_t GetUpTime();

string Request_DumpData();
string Request_SSH(vector<string> args);
string Request_Ping();
// ===========================================


int main(int argc, char** argv) {
	
	// Create the agent
	agent = new SimpleAgent(CUBESAT_AGENT_RASPI_NAME);
	agent->SetLoopPeriod(SLEEP_TIME);
	
	// Add the Raspberry Pi
	raspi = agent->NewDevice<CPU>("raspi");
	raspi->AddProperty<CPU::UTC>(0);
	raspi->AddProperty<CPU::Load>(0);
	raspi->AddProperty<CPU::MemoryUsed>(0);
	raspi->AddProperty<CPU::MaxMemory>(0.5);
	raspi->AddProperty<CPU::BootCount>(0);
	raspi->AddProperty<CPU::UpTime>(0);
	raspi->SetProperty<bool>("connected", false);
	raspi->SetProperty<double>("connection_time", 0);
	
	// Add the camera
	camera = agent->NewDevice<Camera>("camera");
	camera->AddProperty<Camera::UTC>(0);
	camera->AddProperty<Camera::Enabled>(0);
	camera->AddProperty<Camera::PixelWidth>(0);
	camera->AddProperty<Camera::PixelHeight>(0);
	
	// Add custom devices to hold SOH data
	tempsensors = agent->NewDevice<CustomDevice>("agent_temp");
	sunsensors = agent->NewDevice<CustomDevice>("agent_sunsensor");
	heater = agent->NewDevice<CustomDevice>("agent_heater");
	pycubed = agent->NewDevice<CustomDevice>("agent_pycubed");
	switches = agent->NewDevice<CustomDevice>("agent_switch");
	
	agent->FinalizeDevices();
	
	// Add requests
	agent->AddRequest({"dumpdata", "agentdata"}, Request_DumpData, "Prints data collected from other agents");
	agent->AddRequest({"ssh", "command"}, Request_SSH, "Runs a command on the Raspberry Pi");
	agent->AddRequest({"ping", "is_up"}, Request_Ping, "Checks if the Raspberry Pi is up");
	
	agent->DebugPrint();
	
	
	// Run the main loop for this agent
	while ( agent->StartLoop() ) {
		ConnectRaspi();
		GrabSOHData();
	}
	
	delete agent;
	
	
	return 0;
}

void ConnectRaspi() {
	
	const char *ping_cmd = "ping -c1 -W1 " RASPI_HOST " > nul  && echo 'UP' || echo 'DOWN'";
	std::string response;
	const int kNumAttempts = 3;
	
	// Attempt to connect some number of times
	for (int i = 0; i < kNumAttempts; ++i) {
		// Run the ping command to check if the raspi is up
		SystemCall(ping_cmd, response);
		
		// Check the output of the ping command
		if ( response.find("UP") != string::npos ) {
			printf("RaspberryPi is UP\n");
			
			// Set the connection time
			if ( !raspi->GetProperty<bool>("connected") )
				raspi->SetProperty<double>("connection_time", currentmjd());
			
			raspi->SetProperty<bool>("connected", true);
			raspi->SetProperty<CPU::UpTime>(GetUpTime());
			
			return;
		}
		else
			printf("Cannot reach Raspbery Pi. Attempting to reconnect...\n");
	}
	
	raspi->SetProperty<bool>("connected", false);
	raspi->SetProperty<double>("connection_time", 0);
	raspi->SetProperty<CPU::UpTime>(0);
	
	printf("Failed to connect to Raspberry Pi. Will attempt on next cycle.\n");
}

size_t GetUpTime() {
	return utc2unixseconds(currentmjd()) - utc2unixseconds(raspi->GetProperty<double>("connection_time"));
}

void GrabSOHData() {
	static RemoteAgent agent_temp = agent->FindAgent(CUBESAT_AGENT_TEMP_NAME);
	static RemoteAgent agent_sunsensor = agent->FindAgent(CUBESAT_AGENT_SUNSENSOR_NAME);
	static RemoteAgent agent_heater = agent->FindAgent(CUBESAT_AGENT_HEATER_NAME);
	static RemoteAgent agent_switch = agent->FindAgent(CUBESAT_AGENT_SWITCH_NAME);
	static RemoteAgent agent_pycubed = agent->FindAgent(CUBESAT_AGENT_PYCUBED_NAME);
	
	// Get values from agent_temp
	if ( agent_temp.Reconnect() ) {
		auto values = agent_temp.GetValues({"device_tsen_temp_000", "device_tsen_utc_000",
											"device_tsen_temp_001", "device_tsen_utc_001",
											"device_tsen_temp_002", "device_tsen_utc_002",
											"device_tsen_temp_003", "device_tsen_utc_003",
											"device_tsen_temp_004", "device_tsen_utc_004"});
		if ( !values.empty() ) {
			tempsensors->SetProperty<bool>("active", true);
			
			tempsensors->SetProperty<float>("temp_eps", values["device_tsen_temp_000"].nvalue);
			tempsensors->SetProperty<float>("temp_obc", values["device_tsen_temp_001"].nvalue);
			tempsensors->SetProperty<float>("temp_raspi", values["device_tsen_temp_002"].nvalue);
			tempsensors->SetProperty<float>("temp_battery", values["device_tsen_temp_003"].nvalue);
			tempsensors->SetProperty<float>("temp_pycubed", values["device_tsen_temp_004"].nvalue);
			
			tempsensors->SetProperty<double>("utc_eps", values["device_tsen_utc_000"].nvalue);
			tempsensors->SetProperty<double>("utc_obc", values["device_tsen_utc_001"].nvalue);
			tempsensors->SetProperty<double>("utc_raspi", values["device_tsen_utc_002"].nvalue);
			tempsensors->SetProperty<double>("utc_battery", values["device_tsen_utc_003"].nvalue);
			tempsensors->SetProperty<double>("utc_pycubed", values["device_tsen_utc_004"].nvalue);
		}
		else
			tempsensors->SetProperty<bool>("active", false);
	}
	else
		tempsensors->SetProperty<bool>("active", false);
	
	// Get values from agent_sunsensor
	if ( agent_sunsensor.Reconnect() ) {
		auto values = agent_sunsensor.GetValues({"device_ssen_temp_000", "device_ssen_utc_000",
												 "device_ssen_temp_001", "device_ssen_utc_001",
												 "device_ssen_temp_002", "device_ssen_utc_002",
												 "device_ssen_temp_003", "device_ssen_utc_003",
												 "device_ssen_temp_004", "device_ssen_utc_004",
												 "device_ssen_temp_005", "device_ssen_utc_005"});
		if ( !values.empty() ) {
			sunsensors->SetProperty<bool>("active", true);
			
			sunsensors->SetProperty<float>("lux_plusx", values["device_ssen_temp_000"].nvalue);
			sunsensors->SetProperty<float>("lux_minusx", values["device_ssen_temp_001"].nvalue);
			sunsensors->SetProperty<float>("lux_plusy", values["device_ssen_temp_002"].nvalue);
			sunsensors->SetProperty<float>("lux_minusy", values["device_ssen_temp_003"].nvalue);
			sunsensors->SetProperty<float>("lux_plusy", values["device_ssen_temp_004"].nvalue);
			sunsensors->SetProperty<float>("lux_minusy", values["device_ssen_temp_005"].nvalue);
			
			sunsensors->SetProperty<double>("utc_plusx", values["device_ssen_utc_000"].nvalue);
			sunsensors->SetProperty<double>("utc_minusx", values["device_ssen_utc_001"].nvalue);
			sunsensors->SetProperty<double>("utc_plusy", values["device_ssen_utc_002"].nvalue);
			sunsensors->SetProperty<double>("utc_minusy", values["device_ssen_utc_003"].nvalue);
			sunsensors->SetProperty<double>("utc_plusz", values["device_ssen_utc_004"].nvalue);
			sunsensors->SetProperty<double>("utc_minusz", values["device_ssen_utc_005"].nvalue);
		}
		else
			sunsensors->SetProperty<bool>("active", false);
	}
	else
		sunsensors->SetProperty<bool>("active", false);
	
	// Get values from agent_heater
	if ( agent_heater.Reconnect() ) {
		auto values = agent_heater.GetValues({"device_htr_volt_000", "device_htr_utc_000"});
		
		if ( !values.empty() ) {
			heater->SetProperty<bool>("active", true);
			
			heater->SetProperty<bool>("enabled", values["device_htr_volt_000"].nvalue != 0.0);
			heater->SetProperty<double>("utc", values["device_htr_utc_000"].nvalue);
		}
	}
	
	// Get values from agent_switch
	if ( agent_switch.Reconnect() ) {
		auto values = agent_switch.GetValues({"device_swch_volt_000", "device_swch_utc_000",
											  "device_swch_volt_001", "device_swch_utc_001",
											  "device_swch_volt_002", "device_swch_utc_002"});
		if ( !values.empty() ) {
			switches->SetProperty<bool>("active", true);
			
			switches->SetProperty<bool>("sw_temp", values["device_swch_volt_000"].nvalue != 0.0);
			switches->SetProperty<bool>("sw_sunsensor", values["device_swch_volt_001"].nvalue != 0.0);
			switches->SetProperty<bool>("sw_heater", values["device_swch_volt_002"].nvalue != 0.0);
			
			switches->SetProperty<double>("utc_temp", values["device_swch_utc_000"].nvalue);
			switches->SetProperty<double>("utc_sunsensor", values["device_swch_utc_001"].nvalue);
			switches->SetProperty<double>("utc_heater", values["device_swch_utc_002"].nvalue);
		}
		else
			switches->SetProperty<bool>("active", false);
	}
	else
		switches->SetProperty<bool>("active", false);
	
	// Get values from agent_pycubed
	if ( agent_pycubed.Reconnect() ) {
		auto values = agent_switch.GetValues({"device_imu_mag_x_000", "device_imu_mag_y_000", "device_imu_mag_z_000",
											  "device_imu_accel_x_000", "device_imu_accel_y_000", "device_imu_accel_z_000",
											  "device_imu_omega_x_000", "device_imu_omega_y_000", "device_imu_omega_z_000", "device_imu_utc_000",
											  "device_cpu_volt_000", "device_cpu_amp_000", "device_cpu_utc_000",
											  "device_batt_volt_000", "device_batt_amp_000", "device_batt_utc_000"});
		if ( !values.empty() ) {
			pycubed->SetProperty<bool>("active", true);
			
			pycubed->SetProperty<double>("imu_utc", values["device_imu_utc_000"].nvalue);
			pycubed->SetProperty<float>("imu_mag_x", values["device_imu_mag_x_000"].nvalue);
			pycubed->SetProperty<float>("imu_mag_y", values["device_imu_mag_y_000"].nvalue);
			pycubed->SetProperty<float>("imu_mag_z", values["device_imu_mag_z_000"].nvalue);
			pycubed->SetProperty<float>("imu_accel_x", values["device_imu_accel_x_000"].nvalue);
			pycubed->SetProperty<float>("imu_accel_y", values["device_imu_accel_y_000"].nvalue);
			pycubed->SetProperty<float>("imu_accel_z", values["device_imu_accel_z_000"].nvalue);
			pycubed->SetProperty<float>("imu_gyro_x", values["device_imu_omega_000"].nvalue);
			pycubed->SetProperty<float>("imu_gyro_y", values["device_imu_omega_y_000"].nvalue);
			pycubed->SetProperty<float>("imu_gyro_z", values["device_imu_omega_z_000"].nvalue);
			
			pycubed->SetProperty<float>("power_utc", values["device_cpu_utc_000"].nvalue);
			pycubed->SetProperty<float>("sys_voltage", values["device_cpu_volt_000"].nvalue);
			pycubed->SetProperty<float>("sys_current", values["device_cpu_amp_000"].nvalue);
			pycubed->SetProperty<float>("batt_voltage", values["device_batt_volt_000"].nvalue);
			pycubed->SetProperty<float>("batt_current", values["device_batt_amp_000"].nvalue);
		}
		else
			pycubed->SetProperty<bool>("active", false);
	}
	else
		pycubed->SetProperty<bool>("active", false);
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


string Request_DumpData() {
	
	stringstream ss;
	ss << "{";
	
	ss <<	"\"agent_temp\": {";
	ss <<		"\"active\": " << (tempsensors->GetProperty<bool>("active") ? "true" : "false") << ", ";
	ss <<		"\"temp_eps\": " << tempsensors->GetProperty<float>("temp_eps") << ", ";
	ss <<		"\"temp_obc\": " << tempsensors->GetProperty<float>("temp_obc") << ", ";
	ss <<		"\"temp_raspi\": " << tempsensors->GetProperty<float>("temp_raspi") << ", ";
	ss <<		"\"temp_battery\": " << tempsensors->GetProperty<float>("temp_battery") << ", ";
	ss <<		"\"temp_pycubed\": " << tempsensors->GetProperty<float>("temp_pycubed");
	ss <<	"}, ";
	
	ss <<	"\"agent_sunsensor\": {";
	ss <<		"\"active\": " << (sunsensors->GetProperty<bool>("active") ? "true" : "false") << ", ";
	ss <<		"\"ss_plusx\": " << tempsensors->GetProperty<float>("lux_plusx") << ", ";
	ss <<		"\"ss_minusx\": " << tempsensors->GetProperty<float>("lux_minusx") << ", ";
	ss <<		"\"ss_plusy\": " << tempsensors->GetProperty<float>("lux_plusy") << ", ";
	ss <<		"\"ss_minusy\": " << tempsensors->GetProperty<float>("lux_minusy") << ", ";
	ss <<		"\"ss_plusz\": " << tempsensors->GetProperty<float>("lux_plusz") << ", ";
	ss <<		"\"ss_minusz\": " << tempsensors->GetProperty<float>("lux_minusz");
	ss <<	"}, ";
	
	ss <<	"\"agent_pycubed\": {";
	ss <<		"\"active\": " << (pycubed->GetProperty<bool>("active") ? "true" : "false") << ", ";
	ss <<		"\"imu\": {";
	ss <<			"\"accel\": [";
	ss <<				 pycubed->GetProperty<float>("imu_accel_x") << ", ";
	ss <<				 pycubed->GetProperty<float>("imu_accel_y") << ", ";
	ss <<				 pycubed->GetProperty<float>("imu_accel_z");
	ss <<			"], ";
	ss <<			"\"mag\": [";
	ss <<				 pycubed->GetProperty<float>("imu_mag_x") << ", ";
	ss <<				 pycubed->GetProperty<float>("imu_mag_y") << ", ";
	ss <<				 pycubed->GetProperty<float>("imu_mag_z");
	ss <<			"], ";
	ss <<			"\"gyro\": [";
	ss <<				 pycubed->GetProperty<float>("imu_gyro_x") << ", ";
	ss <<				 pycubed->GetProperty<float>("imu_gyro_y") << ", ";
	ss <<				 pycubed->GetProperty<float>("imu_gyro_z");
	ss <<			"]";
	ss <<		"}, ";
	ss <<		"\"power\": {";
	ss <<			"\"batt_voltage\": " << pycubed->GetProperty<float>("batt_voltage") << ", ";
	ss <<			"\"batt_current\": " << pycubed->GetProperty<float>("batt_current") << ", ";
	ss <<			"\"sys_voltage\": " << pycubed->GetProperty<float>("sys_voltage") << ", ";
	ss <<			"\"sys_current\": " << pycubed->GetProperty<float>("sys_current");
	ss <<		"}";
	ss <<	"}, ";
	
	ss <<	"\"agent_switch\": {";
	ss <<		"\"active\": " << (switches->GetProperty<bool>("active") ? "true" : "false") << ", ";
	ss <<		"\"enabled\": [";
	ss <<			(switches->GetProperty<bool>("sw_temp") ? "true" : "false") << ", ";
	ss <<			(switches->GetProperty<bool>("sw_sunsensor") ? "true" : "false") << ", ";
	ss <<			(switches->GetProperty<bool>("sw_heater") ? "true" : "false") << ", ";
	ss <<		"]";
	ss <<	"}, ";
	
	ss <<	"\"agent_heater\": {";
	ss <<		"\"active\": " << (heater->GetProperty<bool>("active") ? "true" : "false") << ", ";
	ss <<		"\"enabled\": " << (heater->GetProperty<bool>("enabled") ? "true" : "false");
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

