# CircuitPython modules
import board
import busio
import digitalio

# Standard modules
import enum

# Internal modules
import radio
import switch


# BeagleBone to PyCubed message types
class RXMessageType(enum.Enum): 
	Handoff = 0
	Startup = 1
	KillRadio = 2
	Enable = 3
	Disable = 4
	ReceiveFile = 5

# PyCubed to BeagleBone message types
class TXMessageType(enum.Enum):
	Shutdown = 0
	SendFile = 1
	IMUData = 2
	GPSData = 3
	TempData = 4
	PowerData = 5


class BeagleBone:
	
	def __init__(self):
		# Set up the UART connection to the BeagleBone
		self.uart = busio.UART(board.TX, board.RX, baudrate=9600)
		self.handoff_callback = None
		self.startup_callback = None
		self.kill_radio_callback = None
	
	def Shutdown(self):
		self.__WriteMessageType(TXMessageType.Shutdown)
	
	
	def SendIMUData(self, bytes):
		self.__WriteMessageType(TXMessageType.IMUData)
		self.uart.write(bytes)
	
	def SendGPSData(self, bytes):
		self.__WriteMessageType(TXMessageType.GPSData)
		self.uart.write(bytes)
		
	def SendTempData(self, bytes):
		self.__WriteMessageType(TXMessageType.TempData)
		self.uart.write(bytes)
	
	def SendPowerData(self, bytes):
		self.__WriteMessageType(TXMessageType.PowerData)
		self.uart.write(bytes)
	
	def PollMessages(self):
		
		while ( self.uart.in_waiting() != 0 ):
			self.__ReadNextMessage()
		
	
	def SendFile(self, src, dest):
		
		self.__WriteMessageType(TXMessageType.SendFile)
		
		#TODO
		# Write destination path (128 bytes)
		# Write file length (64-bit uint)
		# Write file contents
	
	def __WriteMessageType(self, message_type):
		# Create the message buffer
		buf = [message_type]
		
		# Write the message to the UART line
		self.uart.write(buf)
	
	def __ReadString(self, length):
		buf = self.uart.read(length)
		
		# Convert byte array to string
		data_string = ''.join([chr(b) for b in data])
		
		return data_string
	
	def __ReadNextMessage(self):
		### Reads messages from the BeagleBone and acts on them
		# Read the message header
		message_type = self.uart.read(1)[0]
		
		# Handoff Confirmation message
		if ( message_type == RXMessageType.Handoff ):
			
			if ( self.handoff_callback != None ):
				self.handoff_callback()
			else:
				print("Handoff message received, but no callback is set")
		
		# Startup Confirmation message
		if ( message_type == RXMessageType.Startup ):
			
			if ( self.startup_callback != None ):
				self.startup_callback()
			else:
				print("Startup message received, but no callback is set")
			
		# Kill Radio message
		elif ( message_type == RXMessageType.KillRadio ):
			if ( self.kill_radio_callback != None ):
				self.kill_radio_callback()
			else:
				print("Kill radio message received, but no callback is set")
		elif ( message_type == RXMessageType.Enable ):
			# Read device name
			device_name = self.__ReadString(64)
			
			# Enable device
			switch.Enable(device_name)
		elif ( message_type == RXMessageType.Disable ):
			# Read device name
			device_name = self.__ReadString(64)
			
			# Enable device
			switch.Disable(device_name)
		elif ( message_type == RXMessageType.ReceiveFile ):
			# Read file name
			file_name = self.__ReadString(128)
			
			# Read file length
			file_len = int.from_bytes(self.uart.read(4), "big")
			
			# TODO: Read and write file by chunks to save RAM
			# Read file
			file_contents = self.uart.read(file_len)
			
			# Open file
			f = open(file_name, 'wb')
			
			# Write to  the file
			f.write(bytearray(int(i, 8) for i in file_contents))
			
			# Close the file
			f.close()
		else:
			print("Invalid message type '" + str(message_type) + "' received")
		
		
		
	
	



