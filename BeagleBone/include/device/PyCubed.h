
#ifndef CUBESAT_DEVICE_PYCUBED
#define CUBESAT_DEVICE_PYCUBED

#include "device/UARTDevice.h"
#include "device/PyCubedMessage.h"
#include "support/datalib.h"

namespace cubesat {
	
	typedef void (*PyCubedShutdownCallback)();
	typedef void (*PyCubedReceiveFileCallback)(const std::string &file_name);
	
	
	
	/**
	 * @brief Provides access to the PyCubed mainboard
	 */
	class PyCubed : protected UARTDevice {
	public:
		PyCubed() {}
		
		/**
		 * @brief Constructs a PyCubed object using the given bus and device numbers
		 * @param bus The UART bus number to use
		 * @param device The device number to use
		 */
		PyCubed(uint8_t bus, uint8_t device);
		/**
		 * @brief Destructor
		 */
		virtual ~PyCubed();
		
		/**
		 * @brief Sends a message that startup was successful
		 * @return The status of the operation
		 */
		int StartupConfirmation();
		
		
		/**
		 * @brief Sends a handoff confirmation message
		 * @return The status of the operation
		 */
		int Handoff();
		/**
		 * @brief Sends a message to kill the radio
		 * @return The status of the operation
		 */
		int KillRadio();
		
		/**
		 * @brief Sends a message to enable a device
		 * @param name The device name
		 * @return The status of the operation
		 */
		int Enable(const std::string &name);
		
		/**
		 * @brief Sends a message to disable a device
		 * @param name The device name
		 * @return The status of the operation
		 */
		int Disable(const std::string &name);
		
		/**
		 * @brief Sends a file to the PyCubed device
		 * @param source
		 * @param destination
		 * @return The status of the operation
		 */
		int SendFile(const std::string &source, const std::string &destination);
		
		/**
		 * @brief Sets the callback function for when files are received
		 * @param callback The function to call
		 */
		inline void SetReceiveFileCallback(PyCubedReceiveFileCallback callback) {
			this->receive_file_callback = callback;
		}
		
		/**
		 * @brief Sets the callback function for when shutdown is requested
		 * @param callback The function to call
		 */
		inline void SetShutdownCallback(PyCubedShutdownCallback callback) {
			this->shutdown_callback = callback;
		}
		
		
		inline PyCubedIMUInfo GetIMUInfo() const {
			return imu_info;
		}
		
		inline PyCubedGPSInfo GetGPSInfo() const {
			return gps_info;
		}
		
		inline PyCubedTempInfo GetTempInfo() const {
			return temp_info;
		}
		
		inline PyCubedPowerInfo GetPowerInfo() const {
			return power_info;
		}
		
		inline int PopIncoming(PyCubedPacket &packet) {
			if ( incoming_packets.size() == 0 )
				return 0;
			
			packet = incoming_packets.front();
			incoming_packets.pop();
			return packet.content.data.size();
		}
		
		void TelecommandOutboundPacket(PyCubedDataPacket packet);
		
		/**
		 * @brief Polls the PyCubed device for received messages
		 * @return The number of messages received
		 */
		int ReceiveMessages();
		
	private:
		// Callback functions
		PyCubedShutdownCallback shutdown_callback;
		PyCubedReceiveFileCallback receive_file_callback;
		
		PyCubedIMUInfo imu_info; // Latest IMU information
		PyCubedGPSInfo gps_info; // Latest GPS information
		PyCubedTempInfo temp_info;
		PyCubedPowerInfo power_info;
		
		std::queue<PyCubedPacket> incoming_packets;
		
		/**
		 * @brief Receives the next message available.
		 * @return 'true' if there are more messages
		 */
		bool ReceiveNextMessage();
		void ReceiveFile();
		
	};
	
}


#endif
