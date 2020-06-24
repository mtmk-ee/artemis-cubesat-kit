
#include "device/switch.h"

#include <cstdlib>
#include <cstring>
#include <stdio.h>

using namespace std;
using namespace cubesat;

SwitchedDeviceInfo Switch::switch_info[] = {
	{SWITCH_HEATER_NAME, SWITCH_HEATER_KEY},
	{SWITCH_TEMPSENSOR_NAME, SWITCH_TEMPSENSOR_KEY},
	{SWITCH_SUNSENSOR_NAME, SWITCH_SUNSENSOR_KEY},
	{SWITCH_PAYLOAD_NAME, SWITCH_PAYLOAD_KEY},
	{NULL, NULL}
};

Switch::Switch(const char *name) : GPIO(GetSwitchPin(name)), name(name) {
	
	switch_id = GetSwitchID(name);
	
	
	// Set the GPIO to OUTPUT mode
	if ( IsValid() ) {
		GPIO::SetMode(GPIOMode::Output);
	}
}
Switch::~Switch() {
	
}



SwitchState Switch::SetState(SwitchState state) {
	return (SwitchState)GPIO::DigitalWrite((GPIOValue)state);
}

SwitchState Switch::GetState() {
	return (SwitchState)GPIO::DigitalRead();
}


const char* Switch::GetSwitchName(SwitchID switch_id) {
	
	if ( (int)switch_id < 0 || (int)switch_id >= SWITCH_COUNT )
		return NULL;
	else
		return switch_info[(int)switch_id].name;
}
SwitchID Switch::GetSwitchID(const char *switch_name) {
	if ( switch_name == NULL )
		return (SwitchID)-1;
	
	for (int i = 0; i < SWITCH_COUNT; ++i) {
		if ( strcmp(switch_info[i].name, switch_name) == 0 ) {
			return (SwitchID)i;
		}
	}
	
	return (SwitchID)-1;
}
const char* Switch::GetSwitchPinKey(SwitchID switch_id) {
	if ( (int)switch_id < 0 || (int)switch_id >= SWITCH_COUNT )
		return NULL;
	else
		return switch_info[(int)switch_id].key;
}
int Switch::GetSwitchPin(SwitchID switch_id) {
	if ( (int)switch_id < 0 || (int)switch_id >= SWITCH_COUNT )
		return -1;
	else
		return GetPinByKey(switch_info[(int)switch_id].key);
}
int Switch::GetSwitchPin(const char *switch_name) {
	return GetSwitchPin(GetSwitchID(switch_name));
}
