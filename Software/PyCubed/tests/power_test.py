import struct
import time

from pycubed import cubesat

import power


def TestPower(duration):
	print('Testing Power Monitor')
	
	for i in range(duration):
		# Print power usage
		print('=' * 40)
		print('Battery Voltage: {0:.6f} V'.format(power.GetBattVoltage()))
		print('Charge Current: {0:.6f} A'.format(power.GetChargeCurrent()))
		print('System Voltage: {0:.6f} V'.format(power.GetSystemVoltage()))
		print('System Current: {0:.6f} A'.format(power.GetSystemCurrent()))
		
		# Wait for a second
		time.sleep(1)
		
	print('=' * 40)
	print('Test finished')
