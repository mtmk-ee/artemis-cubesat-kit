
#include <iostream>
#include <fstream>
#include  <iomanip>
#include "device/mpu9250.h"

 
using namespace std;
using namespace cubesat;


int main() {
	// Open the MPU 9250 device at address 0x68
    MPU9250 *imu = new MPU9250(0x68, 2);
	
	uint8_t buff[256];
	imu->GetValuesFromRegisters(buff, 0x00, 0xff);
	
	
	for	(int i = 0; i < 16; ++i) {
		cout << " " << hex << i << " ";
	}
	
	cout << endl;
	
	for (int i = 0; i <= 0xff; i++) {
		printf("%2x ", buff[i]);
		
		if ( (i + 1) % 16 == 0 ) {
			printf("\n");
		}
	}
	
	for (int i = 0; i < 100; ++i) {
		uint8_t x_accel_h = imu->GetValueFromRegister(0x3b);
		uint8_t x_accel_l = imu->GetValueFromRegister(0x3c);
		uint16_t x_accel = (x_accel_h << 8) | x_accel_l;
		
		printf("X-accel reading: %d\n", x_accel);
		
		
		usleep(100000);
	}
	
	delete imu;
	
	
	
	
	
    return 0;
}

