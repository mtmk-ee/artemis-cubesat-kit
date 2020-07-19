
#include "utility/SimpleAgent.h"

// Standard headers
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <dirent.h>
#include "device/ADT7311.h"

using namespace std;
using namespace cubesat;


SimpleAgent *agent;

string double_value(vector<string> args) {
	if ( args.size() != 1 ) {
		return "Usage: double_value number";
	}
	return to_string(atof(args[0].c_str()) * 2);
}

int main(int argc, char** argv) {
	
	// Create an agent and set how often it runs
	agent = new SimpleAgent("tempsensor");
	agent->SetLoopPeriod(2);
	
	// Add a 'TemperatureSensor' (built in) device
	Device *my_sensor = agent->NewDevice<TemperatureSensor>("my_sensor");
	
	// Add a property to the device and post it.
	// Posted properties can be displayed in COSMOS Web
	my_sensor->SetProperty<TemperatureSensor::Temperature>(0, true);
	
	// Add a custom property
	my_sensor->SetProperty<int>("address", 0x68);
	
	agent->AddNodeProperty<Node::UTC>(123);
	agent->SetNodeProperty<Node::BatteryCapacity>(987654321.0123456789);
	
	// Let the agent know all the posted properties have been added
	agent->FinalizeDevices();
	
	// Add request functions with aliases
	agent->AddRequest({"double_value", "twice_value"}, double_value, "Doubles a value");
	agent->AddRequest({"half_value", "onehalf_value"},
					  [](vector<string> args) -> string {
							if ( args.size() != 1 ) {
								return "Usage: half_value number";
							}
							return to_string(atof(args[0].c_str()) / 2);
						}
					  , "Halves a value");
	
	
	agent->DebugPrint();
	
	int i = 0;
	
	while ( agent->StartLoop() ) {
		my_sensor->SetProperty<TemperatureSensor::Temperature>(300);
		
		agent->SetNodeProperty<Node::UTC>(currentmjd());
		agent->SetNodeProperty<Node::BatteryCapacity>((float)i);
		
		++i;
	}
	
	delete agent;
	
	
	return 0;
}
