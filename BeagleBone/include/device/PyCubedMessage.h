
#ifndef CUBESAT_DEVICE_PYCUBEDMESSAGE
#define CUBESAT_DEVICE_PYCUBEDMESSAGE

#include <cstdint>
#include <cstdlib>
#include <string>
#include <cstring>
#include "support/datalib.h"

#define PYCUBED_BEAGLEBONE_STATUS_MSGTYPE "BST"
#define PYCUBED_PYCUBED_STATUS_MSGTYPE "PST"
#define PYCUBED_FILE_TRANSFER_MSGTYPE "FTR"
#define PYCUBED_IMU_DATA_MSGTYPE "IMU"
#define PYCUBED_GPS_DATA_MSGTYPE "GPS"
#define PYCUBED_TEMP_DATA_MSGTYPE "TMP"
#define PYCUBED_POWER_DATA_MSGTYPE "PWR"
#define PYCUBED_PACKET_MSGTYPE "PKT"

namespace cubesat {
	
	struct PyCubedPacketHeader {
		uint8_t bytes[6];
		struct {
			unsigned packet_version_number : 3;
			unsigned packet_type : 1;
			unsigned secondary_header_flag : 1;
			unsigned apid : 11;
			unsigned sequence_flags : 2;
			unsigned sequence_count : 14;
			unsigned packet_data_length : 16;
		} fields;
	};
	
	struct PyCubedPacketData
	{
		union {
			uint8_t id_bytes[2];
			uint16_t message_type_id;
		};
		vector<uint8_t> data;
	};
	
	struct PyCubedPacket {
		PyCubedPacketHeader header;
		PyCubedPacketData content;
	};
	
	struct PyCubedDataPacket {
		uint32_t addr;
		uint16_t length;
		vector<uint8_t> data;
		bool corrupted;
	};
	
	
	
	
	
	struct PyCubedIMUInfo {
		float utc;
		rvector mag;
		rvector accel;
		rvector omega;
	};
	
	struct PyCubedGPSInfo {
		float utc;
		bool has_fix;
		float latitude;
		float longitude;
		int fix_quality;
		int sats_used;
		float altitude;
		float speed;
		float azimuth;
		float horizontal_dilution;
	};
	
	struct PyCubedTempInfo {
		float utc;
		float cpu_temp;
		float batt_temp;
	};
	
	struct PyCubedPowerInfo {
		float utc;
		float batt_voltage;
		float batt_current;
		float sys_voltage;
		float sys_current;
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
				Packet,
			};
			
			
			
		}
	}
}


#endif
