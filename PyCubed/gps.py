import struct
from pycubed import cubesat

# SEE https://www.notion.so/GPS-142436b3a4904f2c85024d28902a75f0

# Create GPS
gps = adafruit_gps.GPS(cubesat.uart, debug=False)

gps_data = {
	"utc": 0,
	"has_fix": False,
	"latitude": 0,
	"longitude": 0,
	"fix_quality": 0,
	"satellites": 0,
	"altitude": 0,
	"speed": 0,
	"azimuth": 0,
	"horizontal_dilution": 0,
}

def Update():
	# Update the GPS
	gps.update()
	
	# Make sure the GPS has a fix
	if not gps.has_fix:
		return
	
	
	gps_data["utc"] = 0 # TODO
	gps_data["latitude"] = gps.latitude
	gps_data["longitude"] = gps.longitude
	gps_data["fix_quality"] = gps.fix_quality
	gps_data["satellites"] = gps.satellites
	gps_data["altitude"] = gps.altitude
	gps_data["speed"] = gps.speed # Speed in knots
	gps_data["azimuth"] = gps.azimuth
	gps_data["horizontal_dilution"] = gps.horizontal_dilution
	


def GetTXBytes():
	### Generates a bytearray containing GPS data to be transmitted
	
	# Update the GPS
	Update()
	
	# Convert data to byte array
	bytes = bytearray(struct.pack("f", utc))
	bytes = bytes + bytearray(struct.pack("f", gps_data["latitude"]))
	bytes = bytes + bytearray(struct.pack("f", gps_data["longitude"]))
	bytes = bytes + bytearray(struct.pack("f", gps_data["fix_quality"]))
	bytes = bytes + bytearray(struct.pack("f", gps_data["satellites"]))
	bytes = bytes + bytearray(struct.pack("f", gps_data["altitude"]))
	bytes = bytes + bytearray(struct.pack("f", gps_data["speed"]))
	bytes = bytes + bytearray(struct.pack("f", gps_data["azimuth"]))
	bytes = bytes + bytearray(struct.pack("f", gps_data["horizontal_dilution"]))
	
	return bytes



