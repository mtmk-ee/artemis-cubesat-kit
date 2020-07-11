# CircuitPython modules
import board
import busio
import digitalio

# Standard modules
import enum
import os
from datetime import datetime

# Internal modules
import power
import radio
import switch
import imu
import gps
import temp
import file

from pycubed import cubesat


# Used to sync messages
PREAMBLE = '$--'

# Message type strings
BEAGLEBONE_STATUS_TYPESTR = 'BST'
PYCUBED_STATUS_TYPESTR    = 'PST'
FILE_TRANSFER_TYPESTR     = 'FTR'
IMU_DATA_TYPESTR          = 'IMU'
GPS_DATA_TYPESTR          = 'GPS'
TEMP_DATA_TYPESTR         = 'TMP'
POWER_DATA_TYPESTR        = 'PWR'
PACKET_DATA_TYPESTR       = 'PKT'

# Message format strings
BEAGLEBONE_STATUS_FORMAT = '{},{},{},{},{:n}'




class BeagleBone:
	
	
	
	def __init__(self):
		# Set up the UART connection to the BeagleBone
		self.uart = busio.UART(board.TX, board.RX, baudrate=9600)
		self.startup_flag = False
		self.handoff_flag = False
		self.kill_radio_flag = False
		
	
	def Update(self):
		# Reset flags
		self.startup_flag = False
		self.handoff_flag = False
		self.kill_radio_flag = False
		
		# Clear the UART line by reading messages
		while ( self.uart.in_waiting() != 0 ):
			self.__ReadNextMessage()
			
	def Reconnect(self):
		pass
	
	def Shutdown(self):
		self.__WriteMessage('PST,y')
	
	def SendIMUData(self):
		# Create timestamp
		timestamp = datetime.utcnow().strftime("%Y%m%d%H%M%S")
		
		# Get IMU readings
		accel = imu.GetAccel()
		mag = imu.GetMag()
		omega = imu.GetOmega()
		
		# Write message
		self.__WriteMessage('IMU,{},{:f},{:f},{:f},{:f},{:f},{:f},{:f},{:f},{:f}'.format(timestamp, accel[0], accel[1], accel[2], mag[0], mag[1], mag[2], omega[0], omega[1], omega[2]))
	
	def SendGPSData(self, bytes):
		# Create timestamp
		timestamp = datetime.utcnow().strftime("%Y%m%d%H%M%S")
		
		# Write message
		self.__WriteMessage('GPS,{},{},{:f},{:f},{:d},{:d},{:f},{:f},{:f},{:f}'.format(timestamp, 'y' if gps.has_fix else 'n', gps.latitude, gps.longitude, gps.fix_quality, gps.satellites, gps.altitude, gps.speed, gps.azimuth, gps.horizontal_dilution))
		
	def SendTempData(self, bytes):
		# Create timestamp
		timestamp = datetime.utcnow().strftime("%Y%m%d%H%M%S")
		
		# Write message
		self.__WriteMessage('TMP,{},{:f},{:f}'.format(timestamp, temp.GetMainboardTemp(), temp.getBattboardTemp()))
	
	def SendPowerData(self, bytes):
		# Create timestamp
		timestamp = datetime.utcnow().strftime("%Y%m%d%H%M%S")
		
		# Write message
		self.__WriteMessage('PWR,{},{:f},{:f},{:f},{:f}'.format(timestamp, power.GetBattVoltage(), power.GetChargeCurrent(), power.GetSystemVoltage(), power.GetSystemCurrent()))
	
	
	def SendFile(self, src, dest):
		''' Transfers a file to the BeagleBone. Returns True if successful '''
		try:
			file_len = os.path.getsize(src)
		except:
			return False
		
		file_checksum = file.GetFileChecksum(src)
		
		# Read file
		file_contents = file.ReadString(src)
		if file_contents is None:
			return False
		
		# Write the file transfer message
		self.__WriteMessage('FTR,{},{:d},{:x}'.format(dest, file_len, file_checksum))
		
		# Write the file
		self.uart.write(file_contents.encode('ascii'))
		
		return True
		
	
	def __WriteMessage(self, sentence):
		''' Writes a message to the UART line. The 'sentence' parameter does not
		include the preamble or checksum'''
		
		# Write the preamble
		self.uart.write('$--'.encode('ascii'))
		
		# Write the message
		self.uart.write(sentence.encode('ascii'))
		
		# Write the checksum
		checksum = self.__GetSentenceChecksum(sentence)
		self.uart.write(',{:x}\n'.format(checksum).encode('ascii'))
	
	
	def __ReadSentence(self):
		''' Reads and returns the next sentence as a string, not including the preamble '''
		
		# Make sure there is data in the UART line
		if self.uart.in_waiting() == 0:
			return None
		
		# ============ Search for the preamble ============
		buff = self.uart.read(len(PREAMBLE)) # Read the first 3 characters
		
		while self.uart.in_waiting() > 0:
			
			# Check if the data read corresponds to the correct preamble
			if buff.decode() == PREAMBLE:
				break
			# Otherwise, shift in the next byte
			else:
				buff[0] = buff[1]
				buff[1] = buff[2]
				buff[2] = self.uart.read(1)[0]
		
		# At this point, the remaining data in the UART line should correspond to the message
		# Decode and return the message string
		return self.uart.readline().decode()
	
	def __GetSentenceChecksum(self, sentence):
		'''Calculates the checksum for the given sentence'''
		# TODO
		return 0
		
	
	def __ReadNextMessage(self):
		''' Reads a message from the UART line and acts on it.
		Returns True if a message was successfully parsed. '''
		
		# Read in a sentence from the UART line
		sentence = self.__ReadSentence
		
		# Exit if no sentence was read
		if sentence is None:
			return False
		
		# Get the first three characters of the sentence corresponding to the message type
		msg_type = sentence[:3]
		
		if msg_type == BEAGLEBONE_STATUS_TYPESTR:
			
			# Split the sentence by the comma delimiter
			elements = sentence.split(',')
			
			# Make sure the number of elements is correct
			if elements != 5:
				return False
			
			# Make sure the checksum is correct
			read_checksum = int(elements[4], 16)
			calc_checksum = self.__GetSentenceChecksum(sentence)
			if read_checksum != calc_checksum:
				return False
			
			# Parse the elements and store flags
			self.startup_flag = True if elements[1] == 'y' else False
			self.handoff_flag = True if elements[2] == 'y' else False
			self.kill_radio_flag = True if elements[3] == 'y' else False
			
			return True
		elif msg_type == PACKET_DATA_TYPESTR:
			
			# Split the sentence by the comma delimiter
			head_split = sentence.split(',', 1)
			tail_split = head_split[1].rsplit(',', 1)
			
			checksum = tail_split[1]
			packet = tail_split[0]
				
			# Make sure the checksum is correct
			read_checksum = int(checksum, 16)
			calc_checksum = self.__GetSentenceChecksum(sentence)
			if read_checksum != calc_checksum:
				return False
			
			# Transmit the packet
			b = bytearray()
			b.extend(packet)
			cubesat.radio1.send(b)
		elif msg_type == FILE_TRANSFER_TYPESTR:
			# Split the sentence by the comma delimiter
			elements = sentence.split(',')
			
			# Make sure the number of elements is correct
			if elements != 5:
				return False
			
			# Make sure the checksum is correct
			read_checksum = int(elements[4], 16)
			calc_checksum = self.__GetSentenceChecksum(sentence)
			if read_checksum != calc_checksum:
				return False
			
			# Parse the elements
			dest_path = elements[1]
			file_len = int(elements[2])
			file_checksum = int(elements[3], 16)
			
			# Read the file from the UART line
			file_contents = self.uart.read(file_len)
			
			# Write to the destination file
			try:
				f = open(dest_path, 'w')
				try:
					f.write(file_contents)
					return True
				finally:
					f.close()
			except:
				return False
			
		else:
			# If this case occurs, the message is not valid
			return False
	
	



