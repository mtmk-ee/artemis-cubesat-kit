
#ifndef CUBESAT_DEVICE_TEMPSENSORS
#define CUBESAT_DEVICE_TEMPSENSORS

#include <string>

#define TEMPSENSOR_COUNT 5

#define TEMPSENSOR_EPS_ID 0
#define TEMPSENSOR_OBC_ID 1
#define TEMPSENSOR_PAYLOAD_ID 2
#define TEMPSENSOR_PYCUBED_ID 3
#define TEMPSENSOR_BATT_ID 4

#define TEMPSENSOR_BATT_NAME "temp_battery"
#define TEMPSENSOR_EPS_NAME "temp_eps"
#define TEMPSENSOR_OBC_NAME "temp_obc"
#define TEMPSENSOR_PAYLOAD_NAME "temp_payload"
#define TEMPSENSOR_PYCUBED_NAME "temp_pycubed"

namespace cubesat {
	
	/**
	 * @brief Returns a list of temperature sensor names, ordered by sensor ID
	 * @return 
	 */
	inline const std::string* GetTempSensorNames() {
		static std::string sensor_names[TEMPSENSOR_COUNT];
		static bool populated = false;
		
		if ( !populated ) {
			sensor_names[TEMPSENSOR_BATT_ID] = TEMPSENSOR_BATT_NAME;
			sensor_names[TEMPSENSOR_EPS_ID] = TEMPSENSOR_EPS_NAME;
			sensor_names[TEMPSENSOR_OBC_ID] = TEMPSENSOR_OBC_NAME;
			sensor_names[TEMPSENSOR_PAYLOAD_ID] = TEMPSENSOR_PAYLOAD_NAME;
			sensor_names[TEMPSENSOR_PYCUBED_ID] = TEMPSENSOR_PYCUBED_NAME;
			
			populated = true;
		}
		
		return sensor_names;
	}
	
	/**
	 * @brief Helper function. Converts temperature sensor ID to its string name
	 * @param tempsensor_id The temperature sensor ID
	 * @return The name, or an empty string if the ID was invalid.
	 */
	inline const std::string& GetTempSensorName(int tempsensor_id) {
		if ( tempsensor_id < 0 || tempsensor_id >= TEMPSENSOR_COUNT )
			return "";
		
		return GetTempSensorNames()[tempsensor_id];
	}
	
	/**
	 * @brief Helper function. Converts temperature sensor name to its ID
	 * @param tempsensor_name The temperature sensor name
	 * @return The name, or an empty string if the ID was invalid.
	 */
	inline int GetTempSensorID(const std::string &tempsensor_name) {
		const std::string* sensor_names = GetTempSensorNames();
		
		for (int i = 0; i < TEMPSENSOR_COUNT; ++i) {
			if ( sensor_names[i] == tempsensor_name )
				return i;
		}
		
		return -1;
	}
	
	
	
}


#endif
