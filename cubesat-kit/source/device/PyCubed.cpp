
#include "device/PyCubed.h"

#include <string>

using namespace std;
using namespace cubesat;
using namespace cubesat::detail::pycubed;



int PyCubed::KillRadio() {
	WriteHeader(MakeKillRadioHeader());
}
int PyCubed::Enable(const std::string &name) {
	EnableHeader header = MakeEnableHeader(name);
	
	WriteHeader(header);
	
	// 0 = success
	return 0;
}
int PyCubed::Disable(const std::string &name) {
	DisableHeader header = MakeDisableHeader(name);
	
	WriteHeader(header);
	
	// 0 = success
	return 0;
}
int PyCubed::SendFile(const std::string &source, const std::string &destination) {
	
	// (TODO) Get file length of 'source'
	size_t file_len = 0;
	
	// Write header
	SendFileHeader header = MakeSendFileHeader(destination, file_len);
	
	WriteHeader(header);
	
	
	// Write file using Write(...)
	
	
	// 0 = success
	return 0;
}

int PyCubed::Receive() {
	
	// Counts the number of received messages
	int i = 0;
	
	// Receive messages
	for (; ReceiveNextMessage(); ++i) {
		
	}
	
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
		case RxHeaderType::StateOfHealth:
			ReceiveSOH();
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


