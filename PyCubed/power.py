import struct
from pycubed import cubesat


def GetChargePercent():
	return vbatt


def GetTXBytes():
	### Generates a bytearray containing power data to be sent to the BeagleBone
	
	# Get power info
	vbatt = cubesat.battery_voltage
	ichrg = cubesat.charge_current
	vsys = cubesat.system_voltage
	isys = cubesat.current_draw
	
	bytes = bytearray(struct.pack("f", vbatt))
	bytes = bytes + bytearray(struct.pack("f", ichrg))
	bytes = bytes + bytearray(struct.pack("f", vsys))
	bytes = bytes + bytearray(struct.pack("f", isys))
	
	
	return bytes



