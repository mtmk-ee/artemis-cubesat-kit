# Wishlist of features
# 1. The user is able to determine when they want to collect data
# 2. The user is able to communicate with their payload as they choose (GPIO, UART, SPI, ...)
# 3. The 'cosmos' library obscures the inner workings so that things are more straight-forward to user
#    (for example, cosmos.radio.SendFile() instead of cosmos.agent_pycubed.SendFile())



import cosmos
import time

import Adafruit_BBIO.GPIO as GPIO
import Adafruit_BBIO.UART as UART
from Adafruit_BBIO.SPI import SPI



def DataCollect():
    ## Handles operations necessary to collect data
    
    # Ensure the payload is enabled
    EnablePayload()
    
    # 1. Perform data collection using Adafruit BeagleBone IO libraries
    # 2. Write collected data to file
    # 3. Return file path as string
    

def ShouldDataCollect():
    ## Determines whether data collection should occur
    
    position_data = cosmos.gps.position; # Pulls GPS data from agent_pycubed
    attitude_data = cosmos.imu.attitude; # Pulls IMU data from agent_pycubed
    
    # Calculate based on attitude/position...

def EnablePayload():
    ## Enables payload
    cosmos.switch.Enable("payload") # Sends request to agent_switch to enable payload power

def DisablePayload():
    ## Disables payload
    cosmos.switch.Disable("payload") # Sends request to agent_switch to disable payload power


# Initialize payload
# ...


# main loop
while True:
    
    # Grabs the latest info from COSMOS
    cosmos.Update()
    
    if ShouldDataCollect():
	filepath = DataCollect()
	cosmos.radio.SendFile(filepath) # Sends request to agent_pycubed to transmit file
    
    
    # Sleep for 5 seconds
    time.sleep(5)




