
#ifndef CUBESAT_DEVICE_PYCUBED
#define CUBESAT_DEVICE_PYCUBED

#include "device/UARTDevice.h"
#include "device/PyCubedMessage.h"

namespace cubesat {
	
	
	typedef void (*PyCubedReceiveFileCallback)(const std::string &file_name);
	
	
	/**
	 * @brief Provides access to the PyCubed mainboard
	 */
	class PyCubed : protected UARTDevice {
	public:
		PyCubed();
		
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
		void SetReceiveFileCallback(PyCubedReceiveFileCallback callback);
		
		/**
		 * @brief Polls the PyCubed device for received messages
		 * @return The number of messages received
		 */
		int Receive();
		
	private:
		PyCubedReceiveFileCallback receive_file_callback;
		
		/**
		 * @brief Receives the next message available.
		 * @return 'true' if there are more messages
		 */
		bool ReceiveNextMessage();
		
		void ReceiveSOH();
		void ReceiveFile();
		
		template <detail::pycubed::TxHeaderType T>
		void WriteHeader(const detail::pycubed::TxHeader<T> &header) {
			using namespace detail::pycubed;
			Write(GetHeaderAsBytes(&header), GetHeaderLength(&header));
		}
		
	};
	
}


#endif
