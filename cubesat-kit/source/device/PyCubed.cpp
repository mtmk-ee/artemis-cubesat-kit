
#include "device/PyCubed.h"

#include <string>

using namespace std;
using namespace cubesat;
using namespace cubesat::detail::pycubed;

PyCubed::PyCubed(uint8_t bus, uint8_t device) {
	
}
PyCubed::~PyCubed() {
	
}

int PyCubed::StartupConfirmation() {
	WriteByte((uint8_t)TxHeaderType::Startup);
	return 0;
}
int PyCubed::Handoff() {
	WriteByte((uint8_t)TxHeaderType::Handoff);
	return 0;
}
int PyCubed::KillRadio() {
	WriteByte((uint8_t)TxHeaderType::KillRadio);
	return 0;
}
int PyCubed::Enable(const std::string &name) {
	
	// Copy the name string
	char name_str[64];
	memset(name_str, 0, sizeof(name_str));
	strcpy(name_str, name.c_str());
	
	// Write the message type
	WriteByte((uint8_t)TxHeaderType::Enable);
	
	// Write the device name
	Write((uint8_t*)name_str, sizeof(name_str));
	
	// 0 = success
	return 0;
}
int PyCubed::Disable(const std::string &name) {
	
	// Copy the name string
	char name_str[64];
	memset(name_str, 0, sizeof(name_str));
	strcpy(name_str, name.c_str());
	
	// Write the message type
	WriteByte((uint8_t)TxHeaderType::Disable);
	
	// Write the device name
	Write((uint8_t*)name_str, sizeof(name_str));
	
	// 0 = success
	return 0;
}
int PyCubed::SendFile(const std::string &source, const std::string &destination) {
	
	// (TODO) Get file length of 'source'
	uint64_t file_len = 0;
	
	// Copy the destination string
	char dest_str[128];
	memset(dest_str, 0, sizeof(dest_str));
	strcpy(dest_str, destination.c_str());
	
	// Write message type
	WriteByte((uint8_t)TxHeaderType::SendFile);
	
	// Write file length
	Write((uint8_t*)&file_len, sizeof(file_len));
	
	// Write destination
	Write((uint8_t*)dest_str, sizeof(dest_str));
	
	
	// Write file using Write(...)
	
	
	// 0 = success
	return 0;
}

int PyCubed::ReceiveMessages() {
	
	// Counts the number of received messages
	int i = 0;
	
	// Receive messages
	for (; ReceiveNextMessage(); ++i);
	
	return i;
}

bool PyCubed::ReceiveNextMessage() {
	
	uint8_t buff[128];
	
	size_t status;
	
	// Read header type
	status = Read(buff, 1);
	if ( status != 1 ) {
		return false;
	}
	
	// Check header type
	RxHeaderType header_type = (RxHeaderType)buff[0];
	
	switch ( header_type ) {
		case RxHeaderType::ReceiveFile:
			ReceiveFile();
			break;
		case RxHeaderType::IMUData:
			// Read IMU info
			Read((uint8_t*)&imu_info, sizeof(PyCubedIMUInfo));
			break;
		case RxHeaderType::GPSData:
			// Read IMU info
			Read((uint8_t*)&gps_info, sizeof(PyCubedGPSInfo));
			break;
		case RxHeaderType::BattData:
			// Read IMU info
			Read((uint8_t*)&gps_info, sizeof(PyCubedBattInfo));
			break;
		default:
			printf("Invalid message format!\n");
			break;
	}
	
	// TODO: check if input is awaiting
	return false;
}

void PyCubed::ReceiveFile() {
	char file_name[128];
	
	int status;
	
	// Read destination file name
	status = Read((uint8_t*)file_name, 128);
	if ( status != 128 ) {
		printf("Invalid message format!\n");
		return;
	}
	
	// Read file length
	uint64_t length;
	status = Read((uint8_t*)&length, sizeof(length));
	if ( status != sizeof(length) ) {
		printf("Invalid message format!\n");
		return;
	}
	
	// TODO: copy file from PyCubed into destination
	// ...
	
	// Notify that file was received
	if ( receive_file_callback != nullptr ) {
		receive_file_callback(file_name);
	}
}
