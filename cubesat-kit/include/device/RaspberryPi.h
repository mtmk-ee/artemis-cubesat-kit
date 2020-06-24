
#ifndef CUBESAT_DEVICE_RASPBERRYPI
#define CUBESAT_DEVICE_RASPBERRYPI

#include <string>

#define RASPI_INCOMING_FOLDER "/home/debian/raspi/incoming"
#define RASPI_DESTINATION_FOLDER "/home/pi/incoming"

namespace cubesat {
	
	
	
	class RaspberryPi {
	public:
		
		
		/**
		 * @brief Uses rsync to synchronize source file with destination file
		 * @param src The source file
		 * @param dest The destination file
		 */
		void SendFile(const std::string &src, const std::string &dest);
	};
	
	
}


#endif
