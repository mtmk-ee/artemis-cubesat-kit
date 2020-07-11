import struct
from pycubed import cubesat

def GetBattVoltage():
	return cubesat.battery_voltage

def GetChargeCurrent():
	return cubesat.charge_current # mA

def GetSystemVoltage():
	return cubesat.system_voltage

def GetSystemCurrent():
	return cubesat.current_draw # mA

def GetChargePercent():
	return 0 # TODO


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



