# CircuitPython modules

# PyCubed modules
import enum as enum

import switch
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

import tests

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

BEAGLEBONE_STARTUP_WAIT = 60 # The time to wait for the BeagleBone to boot. If not booted, power off and try again

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
beaglebone_startup_time = 0 # Marks the time that the BeagleBone power was enabled

# ============== Other stuff ==============
def EnsureBeagleBoneStartup():
	''' Cycles power to the BeagleBone until it boots successfully '''
	global bbb
	
	bbb.Update()
	
	while not bbb.startup_flag:
		
		# Enable the BeagleBone
		switch.Enable("beaglebone")
		
		# Store the time the BeagleBone switch was enabled
		beaglebone_startup_time = time.perf_counter()
		
		# Wait for the BeagleBone to issue the startup flag
		while not bbb.startup_flag and (time.perf_counter() - beaglebone_startup_time) < BEAGLEBONE_STARTUP_WAIT:
			bbb.Update()
			time.sleep(1)
		
		# Exit if the BeagleBone booted successfully
		if bbb.startup_flag:
			break
		
		# At this point the BeagleBone did not boot.
		# Disable the BeagleBone
		switch.Disable("beaglebone")
		
		# Wait a few seconds
		time.sleep(5)

# ============== Main Loop ==============

# Make sure the BeagleBone has power before continuing
EnsureBeagleBoneStartup()

while True:
	
	# Nominal power mode
	if power_mode == PowerMode.Nominal:
		
		# Read messages from the BeagleBone
		bbb.Update()
		
		# Check if the radio should be disabled
		if bbb.kill_radio_flag:
			radio.KillRadio()
			bbb.kill_radio_flag = False
		
		# Update sensors
		gps.Update()
		
		# Send data from sensors
		bbb.SendIMUData()
		bbb.SendGPSData()
		bbb.SendTempData()
		bbb.SendPowerData()
		
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
	elif power_mode == PowerMode.EnteringLowPower:
		# Read messages from the BeagleBone
		bbb.Update()
		
		# Check if the BeagleBone is ready for power loss
		if bbb.handoff_flag:
			# Turn off BeagleBone
			switch.Disable("beaglebone")
			
			# Switch power mode
			power_mode = PowerMode.LowPower
		
		# Wait for a bit
		time.sleep(ENTERING_LOW_POWER_MODE_WAIT)
	
	# Low power mode
	elif power_mode == PowerMode.LowPower:
		
		# Check battery health
		batt_charge = power.GetChargePercent()
		
		# Check if power is sufficient to exit low power mode
		if batt_charge > LOW_POWER_EXIT_CHARGE:
			power_mode = PowerMode.ExitingLowPower
			
			# Enable PyCubed devices
			switch.Enable("imu")
			switch.Enable("gps")
			switch.Enable("radio")
			
			# Start up the BeagleBone
			EnsureBeagleBoneStartup()
			power_mode = PowerMode.Nominal
		
		# Wait for a bit
		time.sleep(LOW_POWER_MODE_WAIT)
	
		
	
	
