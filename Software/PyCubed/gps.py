import struct
from pycubed import cubesat

# SEE https://www.notion.so/GPS-142436b3a4904f2c85024d28902a75f0

# Create GPS
gps = adafruit_gps.GPS(cubesat.uart, debug=False)

utc = 0,
has_fix = False;
latitude = 0,
longitude = 0
fix_quality = 0
satellites = 0
altitude = 0
speed = 0
azimuth = 0
horizontal_dilution = 0

def HasFix():
	return has_fix

def Update():
	# Update the GPS
	gps.update()
	
	# Make sure the GPS has a fix
	if not gps.has_fix:
		return
	
	global utc, latitude, longitude, fix_quality, satellites, altitude, speed, azimuth, horizontal_dilution,
	
	utc = 0 # TODO
	latitude = gps.latitude
	longitude = gps.longitude
	fix_quality = gps.fix_quality
	satellites = gps.satellites
	altitude = gps.altitude
	speed = gps.speed # Speed in knots
	azimuth = gps.azimuth
	horizontal_dilution = gps.horizontal_dilution
	


def GetTXBytes():
	### Generates a bytearray containing GPS data to be transmitted
	
	# Update the GPS
	Update()
	
	global utc, latitude, longitude, fix_quality, satellites, altitude, speed, azimuth, horizontal_dilution,
	
	# Convert data to byte array
	bytes = bytearray(struct.pack("f", utc))
	bytes = bytes + bytearray(struct.pack("f", latitude))
	bytes = bytes + bytearray(struct.pack("f", longitude))
	bytes = bytes + bytearray(struct.pack("f", fix_quality))
	bytes = bytes + bytearray(struct.pack("f", satellites))
	bytes = bytes + bytearray(struct.pack("f", altitude))
	bytes = bytes + bytearray(struct.pack("f", speed))
	bytes = bytes + bytearray(struct.pack("f", azimuth))
	bytes = bytes + bytearray(struct.pack("f", horizontal_dilution))
	
	return bytes



