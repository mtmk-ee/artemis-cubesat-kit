
#ifndef CUBESAT_DEVICE_SWITCHED_DEVICES
#define CUBESAT_DEVICE_SWITCHED_DEVICES

#include <string>

// The number of switches
#define SWITCH_COUNT 4

// The switch IDs for various components
#define SWITCH_HEATER_ID 0
#define SWITCH_TEMP_ID 1
#define SWITCH_SS_ID 2
#define SWITCH_PAYLOAD_ID 3

// The switch names for various components
#define SWITCH_HEATER_NAME "heater"
#define SWITCH_TEMP_NAME "temp"
#define SWITCH_SS_NAME "ss"
#define SWITCH_PAYLOAD_NAME "payload"


namespace cubesat {
	
	
	
	/**
	 * @brief Returns a list of switch names, ordered by ID
	 * @return A list of names with length SWITCH_COUNT
	 */
	inline const std::string* GetSwitchNames() {
		static std::string switch_names[SWITCH_COUNT];
		static bool populated = false;
		
		if ( !populated ) {
			switch_names[SWITCH_HEATER_ID] = SWITCH_HEATER_NAME;
			switch_names[SWITCH_TEMP_ID] = SWITCH_TEMP_NAME;
			switch_names[SWITCH_SS_ID] = SWITCH_SS_NAME;
			switch_names[SWITCH_PAYLOAD_ID] = SWITCH_PAYLOAD_NAME;
			
			populated = true;
		}
		
		return switch_names;
	}
	
	/**
	 * @brief Helper function. Converts switch ID to its string name
	 * @param switch_id The switch ID
	 * @return The name, or an empty string if the ID was invalid.
	 */
	inline const std::string& GetSwitchName(int switch_id) {
		if ( switch_id < 0 || switch_id >= SWITCH_COUNT )
			return "";
		
		return GetSwitchNames()[switch_id];
	}
	
	/**
	 * @brief Helper function. Converts switch name to its ID
	 * @param switch_name The switch name
	 * @return The switch ID, or -1 if the name was invalid.
	 */
	inline int GetSwitchID(const std::string &switch_name) {
		const std::string* switch_names = GetSwitchNames();
		
		for (int i = 0; i < SWITCH_COUNT; ++i) {
			if ( switch_names[i] == switch_name )
				return i;
		}
		
		return -1;
	}
	
	
	
}


#endif
