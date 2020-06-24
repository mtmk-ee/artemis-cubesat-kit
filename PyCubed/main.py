# CircuitPython modules

# PyCubed modules
from pycubed import cubesat

# Standard modules
import time

# Internal modules
import beaglebone
import radio
import imu
import gps
import temp
import power

class PowerMode(enum.Enum): 
	Nominal = 0
	EnteringLowPower = 1
	LowPower = 2
	ExitingLowPower = 3

# ============== Constants ==============
LOW_POWER_ENTER_CHARGE = 0.2
LOW_POWER_EXIT_CHARGE = 0.5

NOMINAL_MODE_WAIT = 0.5
ENTERING_LOW_POWER_MODE_WAIT = 0.5
LOW_POWER_MODE_WAIT = 0.5
EXITING_LOW_POWER_MODE_WAIT = 0.5

# ============== Callback Functions ==============

def KillRadioCallback():
	radio.Kill()

def HandoffCallback():
	global power_mode
	
	# Switch to low power mode
	power_mode = PowerMode.LowPower
	
	# Turn off BeagleBone
	switch.Disable("beaglebone")

def StartupCallback():
	global power_mode
	
	# Switch to nominal power mode
	power_mode = PowerMode.Nominal

# ============== Initialization ==============

# Enable switches
switch.Enable("imu")
switch.Enable("gps")
switch.Enable("radio")
switch.Enable("beaglebone")


# ============== Global Variables ==============
# Current power mode (start as Nominal)
power_mode = PowerMode.Nominal

# Create BeagleBone object
bbb = beaglebone.BeagleBone()

# Set callback functions
bbb.kill_radio_callback = KillRadioCallback
bbb.handoff_callback = HandoffCallback
bbb.startup_callback = StartupCallback



# ============== Main Loop ==============
while True:
	
	# Nominal power mode
	if ( power_mode == PowerMode.Nominal ):
		# Read messages from the BeagleBone
		bbb.PollMessages()
		
		# Send IMU data
		bbb.SendIMUData(imu.GetTXBytes())
		
		# Send GPS data
		bbb.SendGPSData(gps.GetTXBytes())
		
		# Send temperature data
		bbb.SendTempData(temp.GetTXBytes())
		
		# Send power data
		bbb.SendPowerData(power.GetTXBytes())
		
		# Check battery health
		batt_charge = power.GetChargePercent()
		
		if batt_charge < LOW_POWER_ENTER_CHARGE:
			# Set to "entering low power" mode
			power_mode = PowerMode.EnteringLowPower
			
			# Disable PyCubed devices
			switch.Disable("imu")
			switch.Disable("gps")
			switch.Disable("radio")
			
			# Request BeagleBone shutdown
			bbb.Shutdown()
		
		# Wait for a bit
		time.sleep(NOMINAL_MODE_WAIT)
		
	# Entering low power mode
	elif ( power_mode == PowerMode.EnteringLowPower ):
		# Read messages from the BeagleBone
		bbb.PollMessages()
		
		# Wait for a bit
		time.sleep(ENTERING_LOW_POWER_MODE_WAIT)
	
	# Low power mode
	elif ( power_mode == PowerMode.LowPower ):
		# Check battery health
		batt_charge = power.GetChargePercent()
		
		# Check if power is sufficient to exit low power mode
		if ( batt_charge > LOW_POWER_EXIT_CHARGE ):
			power_mode = PowerMode.ExitingLowPower
			
			# Enable PyCubed devices
			switch.Enable("imu")
			switch.Enable("gps")
			switch.Enable("radio")
			switch.Enable("beaglebone")
		
		# Wait for a bit
		time.sleep(LOW_POWER_MODE_WAIT)
	
	# Exiting low power mode
	elif ( power_mode == PowerMode.ExitingLowPower ):
		# Read messages from the BeagleBone
		bbb.PollMessages()
		
		# Wait for a bit
		time.sleep(EXITING_LOW_POWER_MODE_WAIT)
		
	
	
