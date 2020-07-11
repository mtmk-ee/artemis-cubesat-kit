import struct
import time

from pycubed import cubesat

import radio


def TestSend(num_packets):
	print('Testing Radio')
	
	# Test TX
	print('=' * 40)
	for i in range(num_packets):
		
		# Send a bunch of dummy info
		print('Sending packet of 8 bytes...')
		radio.Send(bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07]))
		
		# Wait for a second
		time.sleep(1)
	
	print('=' * 40)
	print('Test finished')

def TestReceive(duration):
	# Test RX
	print('=' * 40)
	for i in range(duration):
		# Try to receive a packet
		print('Receiving packet...')
		packet = radio.ReceivePacket()
		
		# Print the packet data, if received
		if packet is None:
			print('No packet received')
		else:
			print('Received raw bytes: {0}'.format(packet))
		
		# Wait for a second
		time.sleep(1)
	
	print('=' * 40)
	print('Test finished')
	
