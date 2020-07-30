import struct
import time

from pycubed import cubesat

import radio
import imu
import gps
import power


def TestRadio():
	print('Testing Radio')
	
	# Test TX
	print('=' * 40)
	for i in range(10):
		print('Sending packet of 8 bytes...')
		radio.Send(bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07]))
	
	# Test RX
	print('=' * 40)
	for i in range(10):
		print('Receiving packet...')
		packet = radio.ReceivePacket()
		if packet is None:
			print('No packet received')
		else:
			print('Received raw bytes: {0}'.format(packet))
	
	print('Test finished')
	

def TestIMU():
	print('Testing IMU')
	
	for i in range(10):
		
		# Get readings
		temp = imu.GetTemp()
		accel = imu.GetAccel()
		mag = imu.GetMag()
		omega = imu.GetOmega()
		
		# Print readings
		print('=' * 40)
		print('Temperature: {0: .6f} C'.format(temp))
		print('Acceleration: ({0: .6f}, {1: .6f}, {2: .6f})'.format(accel[0], accel[1], accel[2]))
		print('Magnetic Field: ({0: .6f}, {1: .6f}, {2: .6f})'.format(mag[0], mag[1], mag[2]))
		print('Angular Velocity: ({0: .6f}, {1: .6f}, {2: .6f})'.format(omega[0], omega[1], omega[2]))
		
		
		# Sleep for a bit
		time.sleep(1)
	
	print('Test finished')
	

def TestGPS():
	print('Testing GPS')
	
	# TODO: Check if GPS is available
	
	
	for i in range(10):
		
		# Check if the GPS has a fix
		if not gps.HasFix():
			print('Waiting for fix')
			continue
		
		# Print GPS data
		print('=' * 40)  # Print a separator line.
		print('Fix Quality: {0:.6f}'.format(gps.fix_quality))
		print('Latitude: {0:.6f} degrees'.format(gps.latitude))
		print('Longitude: {0:.6f} degrees'.format(gps.longitude))
		print('Altitude: {0:.6f} m'.format(gps.altitude))
		print('Speed: {0:.6f} m/s'.format(gps.speed))
		print('Azimuth: {0:.6f} degrees'.format(gps.azimuth))
		
		# Sleep for a bit
		time.sleep(1)
	
	print('Test finished')

def TestPower():
	print('Testing Power Monitor')
	
	for i in range(10):
		# Print power usage
		print('=' * 40)
		print('Battery Voltage: {0:.6f} V'.format(power.GetBattVoltage()))
		print('Charge Current: {0:.6f} A'.format(power.GetChargeCurrent()))
		print('System Voltage: {0:.6f} V'.format(power.GetSystemVoltage()))
		print('System Current: {0:.6f} A'.format(power.GetSystemCurrent()))
		
		time.sleep(1)
	
	print('Test finished')
