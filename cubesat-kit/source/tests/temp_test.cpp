
#include <iostream>
#include <fstream>
#include  <iomanip>
#include "device/temp_sensors.h"
#include "support/configCosmos.h"
#include "device/ADT7311.h"

 
using namespace std;
using namespace cubesat;

void TestSensor(int bus, int addr, int duration = 5) {
	
	// Check if duration is valid
	if ( duration < 1 ) {
		printf("Duration must be greater than zero\n");
		return;
	}
	// Check if bus is valid
	else if ( bus < 0 || bus > 2 ) {
		printf("I2C bus number must be either 0, 1, or 2\n");
		return;
	}
	// Check if device address is valid
	else if ( addr < 0 || addr > 0xFF ) {
		printf("Device address must range from 0 to 0xFF\n");
		return;
	}
	
	
	printf("Testing temperature sensor on bus %d at address %#04x for %d seconds.\n", bus, addr, duration);
	
	// Number of samples to take each second
	const int samples_per_sec = 5;
	
	// Store some statistics
	float temp = 0;
	float total_temp = 0;
	float min_temp = 0;
	float max_temp = 0;
	
	// Create the sensor object
	ADT7311 sensor(bus, addr);
	
	
	for (int i = 0; i < duration * samples_per_sec; ++i) {
		
		// Update sensor
		sensor.ReadState();
		
		// Get temperature
		temp = sensor.GetTemperature();
		printf("Sample %d: Read temperature as %f C\n", i + 1, temp);
		
		// Other stuff
		if ( i == 0 )
			min_temp = max_temp = temp;
		else {
			if ( temp < min_temp )
				min_temp = temp;
			if ( temp > max_temp )
				max_temp = temp;
		}
		
		total_temp += temp;
		
		// Wait for a bit
		COSMOS_SLEEP(1.0f / samples_per_sec);
	}
	
	// Print temperature stats
	printf("Collected %d samples over %d seconds\n", duration * samples_per_sec, duration);
	printf("==========================\n");
	printf("Average: %f C\n", total_temp / (duration * samples_per_sec));
	printf("Minimum: %f C\n", min_temp);
	printf("Maximum: %f C\n", max_temp);
	
	
}


int main(int argc, char ** argv) {
	
	switch ( argc ) {
		case 3:
			TestSensor(atoi(argv[1]), (int)strtol(argv[2], NULL, 0));
			break;
		case 4:
			TestSensor(atoi(argv[1]), (int)strtol(argv[2], NULL, 0), atoi(argv[3]));
			break;
		default:
			printf("Usage: temp_test i2c_bus dev_addr [duration_sec]\n");
			printf("Ex: temp_test 1 0x68\n");
			break;
	}
	
	
	return 0;
}

