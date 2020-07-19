
#include "device/PyCubed.h"

#include <string>

using namespace std;
using namespace cubesat;
using namespace cubesat::detail::pycubed;

PyCubed::PyCubed(uint8_t bus, unsigned int baud) : UARTDevice(bus, baud) {
	
}
PyCubed::~PyCubed() {
	
}

int PyCubed::StartupConfirmation() {
	
	if ( !IsOpen() )
		return -1;
	
	
	char msg[64];
	int checksum = 0; // TODO
	
	int nbytes = sprintf(msg, "$%s,y,n,n,%c", PYCUBED_BEAGLEBONE_STATUS_MSGTYPE, checksum);
	
	Write((uint8_t*)msg, nbytes);
	
	return 0;
}
int PyCubed::Handoff() {
	if ( !IsOpen() )
		return -1;
	
	char msg[64];
	int checksum = 0; // TODO
	
	int nbytes = sprintf(msg, "$%s,n,y,n,%c", PYCUBED_BEAGLEBONE_STATUS_MSGTYPE, checksum);
	
	Write((uint8_t*)msg, nbytes);
	
	return 0;
}
int PyCubed::KillRadio() {
	if ( !IsOpen() )
		return -1;
	
	char msg[64];
	int checksum = 0; // TODO
	
	int nbytes = sprintf(msg, "$%s,n,n,y,%c", PYCUBED_BEAGLEBONE_STATUS_MSGTYPE, checksum);
	
	Write((uint8_t*)msg, nbytes);
	return 0;
}

int PyCubed::ReceiveMessages() {
	if ( !IsOpen() )
		return -1;
	
	// Counts the number of received messages
	int i = 0;
	
	// Receive messages
	for (; ReceiveNextMessage(); ++i);
	
	return i;
}

int PyCubed::TelecommandOutboundPacket(PyCubedDataPacket packet) {
	if ( !IsOpen() )
		return -1;
	
	uint8_t *msg = new uint8_t[32];
	
	int checksum = 0; // TODO
	int len = 0;
	
	// Write the message up until the data
	int num_written = sprintf((char*)msg, "$%s,%d,", PYCUBED_PACKET_MSGTYPE, len);
	Write(msg, len);
	
	// Write the packet data
	Write(packet.data.data(), packet.data.size());
	
	
	// Write the checksum
	num_written += sprintf((char*)msg, ",%02x\n", checksum);
	
	return num_written;
}

bool PyCubed::ReceiveNextMessage() {
	if ( !IsOpen() )
		return false;
	
	uint8_t buff[256];
	
	size_t status;
	
	// Read until the first '$' character is found
	do {
		Read(buff, 1);
	} while ( (char)buff[0] != '$' );
	
		
	// Read message type and the trailiing comma
	status = Read(buff, 4);
	if ( status != 4 ) {
		return false;
	}
	
	
	if ( strncmp((char*)buff, PYCUBED_PACKET_MSGTYPE, 3) == 0 ) {
		
		// Read the length of the packet
		uint8_t length_buff[5];
		uint8_t *p = length_buff;
		
		do {
			Read(p++, 1);
		} while ( (char)*(p - 1) != ',' && (p - length_buff) < 4 );
		
		length_buff[4] = '\0';
		int packet_len = atoi((char*)length_buff);
		
		char last_message_buff[32];
		sprintf(last_message_buff, "PKT of length %d", packet_len);
		strcpy(last_message, last_message_buff);
		
		// Read the packet length
		Read(buff, packet_len);
		
		// Add the packet to the queue
		PyCubedPacket packet;
		packet.content.data = vector<uint8_t>(buff, buff + packet_len);
		incoming_packets.push(packet);
		
		
		// Read the checksum
		uint8_t checksum_buff[4];
		Read(checksum_buff, 4);
		checksum_buff[3] = '\0'; // Change newline to null
		
		int checksum = atoi((char*)checksum_buff);
		
		// TODO: do something with the checksum
		
		
	}
	else {
		
		// Read until a newline character is reached
		uint8_t *p = buff + 4;
		int read_count;
		do {
			read_count = Read(p++, 1);
		} while ( (char)*(p - 1) != '\n' && read_count > 0 );
		
		// Replace the newline with a terminating character
		*(p - 1) = (uint8_t)'\0';
		
		// Store the message
		strncpy(last_message, (char*)buff, 255);
		
		
		if ( strncmp((char*)buff, PYCUBED_PYCUBED_STATUS_MSGTYPE, 3) == 0 ) {
			
			char shutdown_flag;
			int checksum;
			sscanf((char*)buff, "%*s,%c,%x", &shutdown_flag, &checksum);
			
			if ( shutdown_flag == 'y' ) {
				if ( shutdown_callback != NULL )
					shutdown_callback();
				else
					printf("Shutdown flag received, but no callback is set\n");
			}
		}
		else if ( strncmp((char*)buff, PYCUBED_IMU_DATA_MSGTYPE, 3) == 0 ) {
			
			int utc_yyyy;
			int utc_MM;
			int utc_dd;
			int utc_hh;
			int utc_mm;
			int utc_ss;
			float accel_x;
			float accel_y;
			float accel_z;
			float mag_x;
			float mag_y;
			float mag_z;
			float gyro_x;
			float gyro_y;
			float gyro_z;
			int checksum;
			
			sscanf((char*)buff, "%*s,%4d%2d%2d%2d%2d%2d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%x",
				   &utc_yyyy, &utc_MM, &utc_dd, &utc_hh, &utc_mm, &utc_ss,
				   &accel_x, &accel_y, &accel_z,
				   &mag_x, &mag_y, &mag_z,
				   &gyro_x, &gyro_y, &gyro_z,
				   &checksum);
			
			
			
			// Update the IMU info
			imu_info.utc = 0; // TODO
			imu_info.accel.col[0] = accel_x;
			imu_info.accel.col[1] = accel_y;
			imu_info.accel.col[2] = accel_z;
			imu_info.mag.col[0] = mag_x;
			imu_info.mag.col[1] = mag_y;
			imu_info.mag.col[2] = mag_z;
			imu_info.omega.col[0] = gyro_x;
			imu_info.omega.col[1] = gyro_y;
			imu_info.omega.col[2] = gyro_z;
		}
		else if ( strncmp((char*)buff, PYCUBED_GPS_DATA_MSGTYPE, 3) == 0 ) {
			
			int utc_yyyy;
			int utc_MM;
			int utc_dd;
			int utc_hh;
			int utc_mm;
			int utc_ss;
			char has_fix;
			float latitude;
			float longitude;
			int fix_quality;
			int sats_used;
			float altitude;
			float speed;
			float azimuth;
			float horizontal_dilution;
			int checksum;
			
			sscanf((char*)buff, "%*s,%4d%2d%2d%2d%2d%2d,%c,%f,%f,%d,%d,%f,%f,%f,%f,%x",
				   &utc_yyyy, &utc_MM, &utc_dd, &utc_hh, &utc_mm, &utc_ss,
				   &has_fix, &latitude, &longitude, &fix_quality, &sats_used,
				   &altitude, &speed, &azimuth, &horizontal_dilution, &checksum);
			
			gps_info.utc = 0; // TODO
			gps_info.has_fix = has_fix == 'y' ? true : false;
			gps_info.latitude = latitude;
			gps_info.longitude = longitude;
			gps_info.fix_quality = fix_quality;
			gps_info.sats_used = sats_used;
			gps_info.altitude = altitude;
			gps_info.speed = speed;
			gps_info.azimuth = azimuth;
			gps_info.horizontal_dilution = horizontal_dilution;
			
		}
		else if ( strncmp((char*)buff, PYCUBED_TEMP_DATA_MSGTYPE, 3) == 0 ) {
			
			int utc_yyyy;
			int utc_MM;
			int utc_dd;
			int utc_hh;
			int utc_mm;
			int utc_ss;
			float cpu_temp;
			float batt_temp;
			int checksum;
			
			sscanf((char*)buff, "%*s,%4d%2d%2d%2d%2d%2d,%f,%f,%x",
				   &utc_yyyy, &utc_MM, &utc_dd, &utc_hh, &utc_mm, &utc_ss,
				   &cpu_temp, &batt_temp, &checksum);
			
			temp_info.utc = 0; // TODO
			temp_info.cpu_temp = cpu_temp;
			temp_info.batt_temp = batt_temp;
		}
		else if ( strncmp((char*)buff, PYCUBED_POWER_DATA_MSGTYPE, 3) == 0 ) {
			
			
			int utc_yyyy;
			int utc_MM;
			int utc_dd;
			int utc_hh;
			int utc_mm;
			int utc_ss;
			float batt_voltage;
			float batt_current;
			float sys_voltage;
			float sys_current;
			int checksum;
			
			sscanf((char*)buff, "%*s,%4d%2d%2d%2d%2d%2d,%f,%f,%f,%f,%x",
				   &utc_yyyy, &utc_MM, &utc_dd, &utc_hh, &utc_mm, &utc_ss,
				   &batt_voltage, &batt_current,
				   &sys_voltage, &sys_current, &checksum);
			
			power_info.utc = 0; // TODO
			power_info.batt_voltage = batt_voltage;
			power_info.batt_current = batt_current;
			power_info.sys_voltage = sys_voltage;
			power_info.sys_current = sys_current;
		}
		else {
			printf("Invalid PyCubed message received. Message discarded.");
		}
	}
	
	
	
	// TODO: check if input is awaiting
	return false;
}
