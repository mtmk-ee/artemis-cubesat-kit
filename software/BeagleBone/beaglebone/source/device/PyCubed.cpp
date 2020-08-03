
#include "device/PyCubed.h"

#include <string>
#include <sstream>

using namespace std;
using namespace cubesat;

PyCubed::PyCubed(uint8_t bus, unsigned int baud) : UARTDevice(bus, baud) {
	
	// Add default message parsers
	AddMessageParser(PYCUBED_PACKET_MSGTYPE, Parser_PKT);
	
	// Add default message handlers
	AddMessageHandler(PYCUBED_PYCUBED_STATUS_MSGTYPE, Handler_PST, 1);
	AddMessageHandler(PYCUBED_IMU_DATA_MSGTYPE, Handler_IMU, 10);
	AddMessageHandler(PYCUBED_GPS_DATA_MSGTYPE, Handler_GPS, 10);
	AddMessageHandler(PYCUBED_TEMP_DATA_MSGTYPE, Handler_TMP, 3);
	
}
PyCubed::~PyCubed() {
	
}


bool PyCubed::SendMessage(const std::string &message_type_str, const std::vector<std::string> &args) {
	if ( !IsOpen() )
		return false;
	
	
	stringstream message;
	
	// Write the syncword
	message << '$';
	
	// Write the type string
	message << message_type_str;
	
	// Write the arguments
	for (const string &arg : args)
		message << ',' << arg;
	
	// Compute the checksum (TODO)
	int checksum = 0;
	
	// Write the checksum
	message << ',' << std::setfill('0') << std::setw(2) << checksum;
	
	// Write the newline character
	message << '\n';
	
	// Write the entire message
	const char *message_cstr = message.str().c_str();
	Write((uint8_t*)message_cstr, message.str().length());
	
	// Indicate success
	return true;
}

int PyCubed::ReceiveMessages() {
	
	// Make sure the PyCubed device is open
	if ( !IsOpen() )
		return -1;
	
	// Counts the number of received messages
	int num_messages = 0;
	
	// Receive messages
	
	while ( InWaiting() != 0 ) {
		ReceiveNextMessage();
		++num_messages;
	}
	
	return num_messages;
}

int PyCubed::PopIncomingPacket(PyCubedPacket &packet) {
	if ( incoming_packets.size() == 0 )
		return 0;
	
	packet = incoming_packets.front();
	incoming_packets.pop();
	return packet.content.data.size();
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
	
	// Return the total number of bytes written
	return num_written;
}

bool PyCubed::ReceiveNextMessage() {
	// Make sure the PyCubed device is open
	if ( !IsOpen() )
		return false;
	
	uint8_t buff[256];
	
	size_t status;
	
	// Read until the first '$' character is found
	do {
		Read(buff, 1);
	} while ( (char)buff[0] != '$' );
	
		
	// Read message type and the trailing comma
	status = Read(buff, 4);
	if ( status != 4 ) {
		return false;
	}
	
	// Get the message type string
	std::string message_type_str((char*)buff, 0, 3);
	
	// Check if there is a message handler for this message type
	if ( handlers.find(message_type_str) != handlers.end() ) {
		
		MessageHandler &handler = handlers[message_type_str];
		
		// Read until a newline character is reached
		uint8_t *p = buff;
		int read_count;
		do {
			read_count = Read(p++, 1);
		} while ( (char)*(p - 1) != '\n' && read_count > 0 );
		
		// Replace the newline with a terminating character
		*(p - 1) = (uint8_t)'\0';
		
		// This vector holds the argument strings
		std::vector<std::string> argument_strings;
		argument_strings.reserve(handler.num_args);
		
		// Split the message by the comma delimiter
		std::string input((char*)buff, 0, read_count);
		std::istringstream ss(input);
		std::string token;
		
		while(std::getline(ss, token, ','))
			argument_strings.push_back(token);
		
		// Make sure the number of arguments is correct
		if ( argument_strings.size() != handler.num_args + 1 ) {
			printf("Number of arguments received for message type '%s' is incorrect (got %d, expected %d)\n",
				   message_type_str.c_str(), argument_strings.size() - 1, handler.num_args);
			
			return false;
		}
		
		// Compute the checksum (TODO)
		int checksum = 0;
		
		// Remove the checksum from the argument list
		argument_strings.pop_back();
		
		// Call the message handler
		return handler.callback(argument_strings, *this);
	}
	// Check if there is a message parser for this message type
	else if ( parsers.find(message_type_str) != parsers.end() ) {
		return parsers[message_type_str].callback(*(UARTDevice*)this, *this);
	}
	// At this point, the message is not supported
	else {
		printf("PyCubed message type '%s' is not supported.\n", message_type_str.c_str());
		return false;
	}
}

bool PyCubed::Parser_PKT(UARTDevice &uart, PyCubed &pycubed) {
	uint8_t buff[256];
	
	// Read the length of the packet
	uint8_t length_buff[5];
	uint8_t *p = length_buff;
	
	do {
		uart.Read(p++, 1);
	} while ( (char)*(p - 1) != ',' && (p - length_buff) < 4 );
	
	// Convert the packet length string to an integer
	length_buff[4] = '\0';
	int packet_len = atoi((char*)length_buff);
	
	// Print a debug string
	printf("Received packet of length %d\n", packet_len);
	
	// Read the packet
	uart.Read(buff, packet_len);
	
	// Add the packet to the incoming packet queue
	PyCubedPacket packet;
	packet.content.data = vector<uint8_t>(buff, buff + packet_len);
	pycubed.incoming_packets.push(packet);
	
	
	// Read the checksum
	uint8_t checksum_buff[4];
	checksum_buff[3] = '\0'; // Change newline to null
	
	int checksum = atoi((char*)checksum_buff + 1);
	
	// TODO: do something with the checksum
	
	return true;
}
bool PyCubed::Handler_PST(std::vector<std::string> args, PyCubed &pycubed) {
	
	// Store timestamp (TODO)
	bool shutdown_requested = args[0][0] == 'y';
	
	if ( shutdown_requested ) {
		if ( pycubed.shutdown_callback != nullptr )
			pycubed.shutdown_callback();
		else
			printf("Shutdown signal received, but no callback is set\n");
	}
	
	// Indicate success
	return true;
}
bool PyCubed::Handler_IMU(std::vector<std::string> args, PyCubed &pycubed) {
	
	// Store timestamp (TODO)
	string timestamp = args[0];
	
	// Store vector values
	pycubed.imu_info.acceleration.x = atof(args[1].c_str());
	pycubed.imu_info.acceleration.y = atof(args[2].c_str());
	pycubed.imu_info.acceleration.z = atof(args[3].c_str());
	
	pycubed.imu_info.magnetometer.x = atof(args[4].c_str());
	pycubed.imu_info.magnetometer.y = atof(args[5].c_str());
	pycubed.imu_info.magnetometer.z = atof(args[6].c_str());
	
	pycubed.imu_info.gyroscope.x = atof(args[7].c_str());
	pycubed.imu_info.gyroscope.y = atof(args[8].c_str());
	pycubed.imu_info.gyroscope.z = atof(args[9].c_str());
	
	// Indicate success
	return true;
}
bool PyCubed::Handler_GPS(std::vector<std::string> args, PyCubed &pycubed) {
	
	// Store timestamp (TODO)
	string timestamp = args[0];
	
	// Check if the GPS has a fix
	pycubed.gps_info.has_fix = args[1][0] == 'y';
	
	// Only store new information if the GPS has a fix
	if ( pycubed.gps_info.has_fix ) {
		pycubed.gps_info.latitude = atof(args[2].c_str());
		pycubed.gps_info.longitude = atof(args[3].c_str());
		pycubed.gps_info.fix_quality = atoi(args[4].c_str());
		pycubed.gps_info.sats_used = atoi(args[5].c_str());
		pycubed.gps_info.altitude = atof(args[6].c_str());
		pycubed.gps_info.speed = atof(args[7].c_str());
		pycubed.gps_info.azimuth = atof(args[8].c_str());
		pycubed.gps_info.horizontal_dilution = atof(args[9].c_str());
	}
	
	// Indicate success
	return true;
}
bool PyCubed::Handler_TMP(std::vector<std::string> args, PyCubed &pycubed) {
	
	// Store timestamp (TODO)
	string timestamp = args[0];
	
	// Store temperature readings
	pycubed.temp_info.cpu_temp = atof(args[1].c_str());
	pycubed.temp_info.batt_temp = atof(args[2].c_str());
	
	return true;
}
bool PyCubed::Handler_PWR(std::vector<std::string> args, PyCubed &pycubed) {
	
	// Store timestamp (TODO)
	string timestamp = args[0];
	
	// Store power readings
	pycubed.power_info.batt_voltage = atof(args[1].c_str());
	pycubed.power_info.batt_current = atof(args[2].c_str());
	pycubed.power_info.sys_voltage = atof(args[3].c_str());
	pycubed.power_info.sys_current = atof(args[4].c_str());
	
	return true;
}


