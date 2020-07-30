import struct
from pycubed import cubesat


def GetMainboardTemp():
	return cubesat.temp

def GetBattboardTemp():
	return 10 # TODO


def GetTXBytes():
	### Generates a bytearray containing temperature data to be sent to the BeagleBone
	
	# Get temperature readings
	imu_temp = cubesat.temp
	batt_temp = GetBattboardTemp()
	
	# Convert temperatures to bytearray
	bytes = bytearray(struct.pack("f", imu_temp))
	bytes = bytes + bytearray(struct.pack("f", batt_temp))
	
	return bytes



