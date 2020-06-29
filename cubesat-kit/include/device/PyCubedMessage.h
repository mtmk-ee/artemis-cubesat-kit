
#ifndef CUBESAT_DEVICE_PYCUBEDMESSAGE
#define CUBESAT_DEVICE_PYCUBEDMESSAGE

#include <cstdint>
#include <cstdlib>
#include <string>
#include <cstring>
#include "support/datalib.h"

namespace cubesat {
	
	
	
	struct PyCubedIMUInfo {
		float utc;
		float temp;
		rvector mag;
		rvector bdot;
		rvector accel;
		rvector alpha;
		avector euler;
		rvector omega;
		quaternion theta;
	};
	
	struct PyCubedGPSInfo {
		rvector geocs;
		rvector geocv;
		int sats_used;
		int sats_visible;
		int time_status;
		int position_type;
		int solution_status;
	};
	
	struct PyCubedBattInfo {
		float timestamp;
		float temp;
		float amp;
		float volt;
		float power;
		float charge;
		float percentage;
		float time_remaining;
	};
	
	namespace detail {
		namespace pycubed {
			
			
			// A command sent to the PyCubed
			enum class TxHeaderType : uint8_t {
				Handoff, // Notifies PyCubed that it can take over as controller
				Startup,
				KillRadio, // Tells PyCubed to disable the radio
				Enable,
				Disable,
				SendFile, // Send file to PyCubed
			};
			
			// A command received from the PyCubed
			enum class RxHeaderType : uint8_t {
				Shutdown,
				ReceiveFile, // Receive file from PyCubed
				IMUData,
				GPSData,
				BattData,
				PowerData,
			};
			
			
			
		}
	}
}


#endif
