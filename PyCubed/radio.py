import board
import busio
import digitalio

from pycubed import cubesat


# Check if the radio was found
if ( !cubesat.hardware['Radio1'] ):
	print("No radio found onboard")
	exit()




def SetTXPower(power):
	cubesat.radio1.tx_power = power

def SetFrequencyMHz(freq):
	cubesat.radio1.frequency_mhz = freq;



# Configure radio (TODO: check values)
SetTXPower(23)
SetFrequencyMHz(431.0)


def	ReceivePacket(blocking = False):
	packet = cubesat.radio1.receive()
	if packet is None:
		return None
	else:
		print("Received raw bytes: {0}".format(packet))
		return packet
	

def Send(bytes):
	cubesat.radio1.send(bytes)

def SendFile(file_name):
	pass #TODO
	




