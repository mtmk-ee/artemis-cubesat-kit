import struct
from pycubed import cubesat


def GetTXBytes():
	### Generates a bytearray containing IMU data to be sent to the BeagleBone
	
	# Get IMU readings
	accel = cubesat.acceleration
	mag = cubesat.magnetic
	omega = cubesat.gyro
	
	# Convert acceleration to bytearray
	bytes = bytearray(struct.pack("f", accel[0]))
	bytes = bytes + bytearray(struct.pack("f", accel[1]))
	bytes = bytes + bytearray(struct.pack("f", accel[2]))
	
	# Convert magnetic field reading to bytearray
	bytes = bytes + bytearray(struct.pack("f", mag[0]))
	bytes = bytes + bytearray(struct.pack("f", mag[1]))
	bytes = bytes + bytearray(struct.pack("f", mag[2]))
	
	# Convert angular velocity to bytearray
	bytes = bytes + bytearray(struct.pack("f", omega[0]))
	bytes = bytes + bytearray(struct.pack("f", omega[1]))
	bytes = bytes + bytearray(struct.pack("f", omega[2]))
	
	return bytes



