
#ifndef CUBESAT_DEVICE_PYCUBEDMESSAGE
#define CUBESAT_DEVICE_PYCUBEDMESSAGE

#include <cstdint>
#include <cstdlib>
#include <string>
#include <cstring>

namespace cubesat {
	namespace detail {
		namespace pycubed {
			
			// A command sent to the PyCubed
			enum class TxHeaderType : uint8_t {
				Handoff, // Notifies PyCubed that it can take over as controller
				KillRadio, // Tells PyCubed to disable the radio
				Enable,
				Disable,
				SendFile, // Send file to PyCubed
			};
			
			// A command received from the PyCubed
			enum class RxHeaderType : uint8_t {
				ReceiveFile, // Receive file from PyCubed
				StateOfHealth, // Receive state of health
			};
			
			template <TxHeaderType T>
			struct TxHeader {};
			
			template <RxHeaderType T>
			struct RxHeader {};
			
			
			// ========================== TX Headers ==========================
			template<>
			struct TxHeader<TxHeaderType::KillRadio> {
				TxHeaderType type;
			};
			using KillRadioHeader = TxHeader<TxHeaderType::KillRadio>;
			
			KillRadioHeader MakeKillRadioHeader() {
				return KillRadioHeader {TxHeaderType::KillRadio};
			}
			
			
			template<>
			struct TxHeader<TxHeaderType::Enable> {
				TxHeaderType type;
				char name[128];
			};
			using EnableHeader = TxHeader<TxHeaderType::Enable>;
			
			EnableHeader MakeEnableHeader(const std::string &name) {
				EnableHeader header;
				header.type = TxHeaderType::Enable;
				memset(header.name, 0, sizeof(header.name));
				strcpy(header.name, name.c_str());
				
				return header;
			}
			
			template<>
			struct TxHeader<TxHeaderType::Disable> {
				TxHeaderType type;
				char name[128];
			};
			using DisableHeader = TxHeader<TxHeaderType::Disable>;
			
			DisableHeader MakeDisableHeader(const std::string &name) {
				DisableHeader header;
				header.type = TxHeaderType::Disable;
				memset(header.name, 0, sizeof(header.name));
				strcpy(header.name, name.c_str());
				
				return header;
			}
			
			
			template<>
			struct TxHeader<TxHeaderType::SendFile> {
				TxHeaderType type;
				char destination[128]; // Destination file name
				uint64_t length; // Length of file in bytes to send
			};
			using SendFileHeader = TxHeader<TxHeaderType::SendFile>;
			
			SendFileHeader MakeSendFileHeader(const std::string &dest, uint64_t length) {
				SendFileHeader header;
				header.type = TxHeaderType::SendFile;
				header.length = length;
				memset(header.destination, 0, sizeof(header.destination));
				strcpy(header.destination, dest.c_str());
				
				return header;
			}
			
			
			// ========================== RX Headers ==========================
			template<>
			struct RxHeader<RxHeaderType::ReceiveFile> {
				RxHeaderType type;
				char destination[128]; // Destination file name
				uint64_t length; // Length of file in bytes to receive
			};
			
			template<>
			struct RxHeader<RxHeaderType::StateOfHealth> {
				RxHeaderType type;
			};
			
			
			
			// ========================== Helpers ==========================
			template <TxHeaderType T>
			inline uint8_t* GetHeaderAsBytes(TxHeader<T> *header) {
				return static_cast<uint8_t*>(header);
			}
			
			template <TxHeaderType T>
			inline size_t GetHeaderLength(TxHeader<T> *header = nullptr) {
				return sizeof(TxHeader<T>);
			}
			
			
			
		}
	}
}


#endif
