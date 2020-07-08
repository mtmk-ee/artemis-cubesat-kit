from collections import namedtuple

from agent import Agent
from sunsensor import SunSensor
from gps import GPS
from heater import Heater
from imu import IMU
from beaglebone import BeagleBone
from tempsensor import TempSensor

# COSMOS Constants
COSMOS_NODE_NAME = 'cubesat'
COSMOS_AGENT_TEMPSENSOR_NAME = 'temp'
COSMOS_AGENT_SUNSENSOR_NAME = 'sunsensor'
COSMOS_AGENT_HEATER_NAME = 'heater'
COSMOS_AGENT_PYCUBED_NAME = 'pycubed'
COSMOS_AGENT_RASPI_NAME = 'raspi'
COSMOS_AGENT_SWITCH_NAME = 'switch'


_SunSensorTuple = namedtuple('_SunSensorTuple', ['plusx', 'minusx', 'plusy', 'minusy', 'plusz', 'minusz'])
_TempSensorTuple = namedtuple('_TempSensorTuple', ['eps', 'obc', 'payload', 'battery', 'pycubed'])


class CubeSat:
    """The main class for interacting with the CubeSat Kit"""
    def __init__(self):
        
        # Set up components
        self._beaglebone = BeagleBone()
        self._imu = IMU()
        self._gps = GPS()
        self._heater = Heater()
        self._sunsensor = _SunSensorTuple(plusx=SunSensor('ss_plusx'),
                                          minusx=SunSensor('ss_minusx'),
                                          plusy=SunSensor('ss_plusx'),
                                          minusy=SunSensor('ss_minusx'),
                                          plusz=SunSensor('ss_plusx'),
                                          minusz=SunSensor('ss_minusx'))
        self._tempsensor = _TempSensorTuple(eps=TempSensor('temp_eps'),
                                            obc=TempSensor('temp_obc'),
                                            payload=TempSensor('temp_payload'),
                                            battery=TempSensor('temp_battery'),
                                            pycubed=TempSensor('temp_pycubed'))

        # Create Agent objects
        self.agent_temp = Agent(COSMOS_AGENT_TEMPSENSOR_NAME, COSMOS_NODE_NAME)
        self.agent_sunsensor = Agent(COSMOS_AGENT_SUNSENSOR_NAME, COSMOS_NODE_NAME)
        self.agent_heater = Agent(COSMOS_AGENT_HEATER_NAME, COSMOS_NODE_NAME)
        self.agent_pycubed = Agent(COSMOS_AGENT_PYCUBED_NAME, COSMOS_NODE_NAME)
        self.agent_raspi = Agent(COSMOS_AGENT_RASPI_NAME, COSMOS_NODE_NAME)
        self.agent_switch = Agent(COSMOS_AGENT_SWITCH_NAME, COSMOS_NODE_NAME)
        
    def update(self):
        """Updates the CubeSat with the latest information"""
        
        # Update the BeagleBone
        self.beaglebone.update()
    
        # Get the PyCubed SOH and update the sensor readings
        pycubed_soh = self.agent_pycubed.soh
    
        self.imu.parse_soh_string(pycubed_soh)
        self.gps.parse_soh_string(pycubed_soh)
        self.tempsensor.pycubed.parse_soh_string(pycubed_soh)
        self.tempsensor.battery.parse_soh_string(pycubed_soh)
    
        # Update temperature sensors
        temp_soh = self.agent_temp.soh
        self.tempsensor.eps.parse_soh_string(temp_soh)
        self.tempsensor.obc.parse_soh_string(temp_soh)
        self.tempsensor.payload.parse_soh_string(temp_soh)
    
        # Update sun sensors
        ss_soh = self.agent_sunsensor.soh
        self.sunsensor.plusx.parse_soh_string(ss_soh)
    
        # Update heater
        heater_soh = self.agent_heater.soh
        self.heater.parse_soh_string(heater_soh)
    
    @property
    def set_soh(self):
        """Sets the State of Health string for your payload"""
        pass
    
    @property
    def beaglebone(self):
        return self._beaglebone

    @property
    def imu(self):
        return self._imu

    @property
    def gps(self):
        return self._gps

    @property
    def tempsensor(self):
        return self._tempsensor

    @property
    def sunsensor(self):
        return self._sunsensor
    
    @property
    def heater(self):
        return self._heater
    

