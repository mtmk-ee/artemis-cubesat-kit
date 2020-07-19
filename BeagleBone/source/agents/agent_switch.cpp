/*
 * agent_switch.cpp
 * 
 * Agent which handles switching of power lines
 */ 

#include "utility/SimpleAgent.h"
#include "device/switch.h"

#include <iostream>
#include <fstream>
#include <sstream>

// The interval between iterations in the run_agent() function
#define SLEEP_TIME 1

using namespace std;
using namespace cubesat;


// A convenient struct for storing switch information
struct SwitchInfo {
	Switch *sw; // The switch device
	const char *gpio_key;
	
	int dindex; // COSMOS device index
	int cindex; // COSMOS component index
	int pindex; // COSMOS piece index
} switches[SWITCH_COUNT]; // Array of SwitchInfo objects, one for each controllable switch


// The agent object which allows for communication with COSMOS
SimpleAgent *agent;
Device *switch_temp, *switch_sunsensor, *switch_heater;

// =========== Function Prototypes ===========

/**
 * @brief Sets up the agent. Prints a message and exits with code 1 if an error occurs during setup.
 */
void InitAgent();

/**
 * @brief Main loop for this agent.
 */
void run_agent();

/**
 * @brief Adds a switch piece to the COSMOS namespace.
 * @param switch_id The switch ID
 */
void add_switch_piece(int switch_id);

/**
 * @brief Initializes a PDU switch
 * @param device The switch device
 */
void InitSwitch(Device *device);
/**
 * @brief UpdateSwitch Updates a switch
 * @param device The switch device
 */
void UpdateSwitch(Device *device);


/**
 * @brief Attempts to set the state of a switch (on/off)
 * @param switch_id The switch ID number. See the file device/switched_devices.h for more info
 * @param enabled True for on, false for off
 */
void SetSwitchState(int switch_id, bool enabled);


// Request callbacks

string Request_Switch(vector<string> args);
string Request_List();
// ===========================================






int main(int argc, char** argv) {
	
	agent = new SimpleAgent(CUBESAT_AGENT_SWITCH_NAME);
	agent->SetLoopPeriod(SLEEP_TIME);
	
	switch_temp = agent->NewDevice<Switch>(SWITCH_TEMPSENSOR_NAME);
	switch_sunsensor = agent->NewDevice<Switch>(SWITCH_SUNSENSOR_NAME);
	switch_heater = agent->NewDevice<Switch>(SWITCH_HEATER_NAME);
	
	switch_temp->AddProperty<Switch::Enabled>(false);
	switch_sunsensor->AddProperty<Switch::Enabled>(false);
	switch_heater->AddProperty<Switch::Enabled>(false);
	
	switch_temp->AddProperty<Switch::Voltage>(0);
	switch_sunsensor->AddProperty<Switch::Voltage>(0);
	switch_heater->AddProperty<Switch::Voltage>(0);
	
	agent->FinalizeDevices();
	
	agent->AddRequest({"switch", "state", "set", "get"}, Request_Switch, "Gets or sets the status of a switch", "Usage: switch switch_name [on | off]");
	agent->AddRequest("list", Request_List, "Lists available switches");
	
	agent->DebugPrint();
	
	
	// Initialize the switches
	InitSwitch(switch_temp);
	InitSwitch(switch_sunsensor);
	InitSwitch(switch_heater);
	
	// Run the main loop for this agent
	while ( agent->StartLoop() ) {
		UpdateSwitch(switch_temp);
		UpdateSwitch(switch_sunsensor);
		UpdateSwitch(switch_heater);
	}
	
	
	delete agent;
	
	
	return 0;
}


void UpdateSwitch(Device *device) {
	PDUSwitch *pdu_switch = device->GetProperty<PDUSwitch*>("handler");
	
	device->Timestamp<Switch>();
	device->SetProperty<Switch::Enabled>(pdu_switch->GetState() == SwitchState::On);
	device->SetProperty<Switch::Voltage>(pdu_switch->GetState() == SwitchState::On ? 3.3 : 0);
}

void InitSwitch(Device *device) {
	
	PDUSwitch *pdu_switch = new PDUSwitch(device->GetName().c_str());
	if ( !pdu_switch->IsValid() )
		printf("Failed to add switch '%s'\n", device->GetName().c_str());
	else
		printf("Added switch '%s'\n", device->GetName().c_str());
	
	device->SetProperty<PDUSwitch*>("handler", pdu_switch);
}

void SetSwitchState(Device *device, bool enabled) {
	
	PDUSwitch *pdu_switch = device->GetProperty<PDUSwitch*>("handler");
	
	// Turn the switch on or off
	bool old_state = pdu_switch->GetState() == SwitchState::On;
	bool new_state = pdu_switch->SetState(enabled ? SwitchState::On : SwitchState::Off) == SwitchState::On;
	
	if ( new_state != enabled )
		printf("Failed to set switch '%s' to '%s'\n",
			   device->GetName().c_str(), enabled ? "ON" : "OFF");
	else if ( new_state != old_state )
		printf("Set switch '%s' to '%s'\n",
			   device->GetName().c_str(), enabled ? "ON" : "OFF");
	
	// Update switch state
	device->Timestamp<Switch>();
	device->SetProperty<Switch::Enabled>(new_state);
	device->SetProperty<Switch::Voltage>(new_state ? 3.3 : 0);
}

string ToLowercase(std::string input) {
	for (size_t i = 0; i < input.length(); ++i)
		input[i] = tolower(input[i]);
	
	return input;
}

string Request_Switch(vector<string> args) {
	
	bool change_state = false;
	bool state = true;
	
	if ( args.size() == 1 );
	else if ( args.size() == 2 ) {
		change_state = true;
		args[1] = ToLowercase(args[1]);
		
		if ( args[1] == "on" || args[1] == "yes" )
			state = true;
		else if ( args[1] == "off" || args[1] == "no" )
			state = false;
		else
			return "Usage: switch switch_name [on | off]";
	}
	else
		return "Usage: switch switch_name [on | off]";
	
	// Check if all switches should be set
	if ( args[0] == "all" ) {
		SetSwitchState(switch_temp, state);
		SetSwitchState(switch_sunsensor, state);
		SetSwitchState(switch_heater, state);
		return "OK";
	}
	
	Device *device;
	if ( (device = agent->GetDevice(args[0])) == nullptr )
		return "No matching switch";
	
	if ( change_state ) {
		SetSwitchState(device, state);
		return "OK";
	}
	else {
		PDUSwitch *pdu_switch = device->GetProperty<PDUSwitch*>("handler");
		
		stringstream ss;
		ss <<	"{";
		ss <<		"\"enabled\": " << (device->GetProperty<Switch::Voltage>() == 0 ? "false" : "true") << ", ";
		ss <<		"\"gpio_pin_key\": \"" << pdu_switch->GetPinKey() << "\", ";
		ss <<		"\"gpio_pin_number\": " << pdu_switch->GetPinNumber();
		ss <<	"}";
		
		return ss.str();
	}
}

string Request_List() {
	stringstream ss;
	
	auto add_switch_info = [&ss](Device *device) {
		PDUSwitch *pdu_switch = device->GetProperty<PDUSwitch*>("handler");
		ss <<	"\"" << device->GetName() << "\": {";
		ss <<		"\"enabled\": " << (device->GetProperty<Switch::Voltage>() == 0 ? "false" : "true") << ", ";
		ss <<		"\"gpio_pin_key\": \"" << pdu_switch->GetPinKey() << "\", ";
		ss <<		"\"gpio_pin_number\": " << pdu_switch->GetPinNumber();
		ss <<	"}, ";
	};
	ss << "{";
	add_switch_info(switch_temp);
	add_switch_info(switch_sunsensor);
	add_switch_info(switch_heater);
	ss << "}";
	
	return ss.str();
}
