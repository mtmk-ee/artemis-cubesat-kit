from collections import namedtuple

import json
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

def _byteify(dictionary):
    """Used to convert the dictionary produced by json.loads() to ASCII instead of Unicode"""
    if isinstance(dictionary, dict):
        return {_byteify(key): _byteify(value)
                for key, value in dictionary.iteritems()}
    elif isinstance(dictionary, list):
        return [_byteify(element) for element in dictionary]
    elif isinstance(dictionary, unicode):
        return dictionary.encode('utf-8')
    else:
        return dictionary

class CubeSat:
    """The main class for interacting with the CubeSat Kit"""
    def __init__(self):
        
        # Set up components
        self._beaglebone = BeagleBone()
        self._imu = IMU()
        self._gps = GPS()
        self._heater = Heater(self)
        self._sunsensor = _SunSensorTuple(plusx=SunSensor('ss_plusx'),
                                          minusx=SunSensor('ss_minusx'),
                                          plusy=SunSensor('ss_plusx'),
                                          minusy=SunSensor('ss_minusx'),
                                          plusz=SunSensor('ss_plusx'),
                                          minusz=SunSensor('ss_minusx'))
        self._tempsensor = _TempSensorTuple(eps=TempSensor('temp_eps'),
                                            obc=TempSensor('temp_obc'),
                                            payload=TempSensor('temp_raspi'),
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
        
        # Get a JSON string from agent_raspi holding agent SOH info
        data_str = self.agent_raspi.request('dumpdata')
        
        # Convert the JSON string into a dictionary
        data_json = {}
        try:
            data_json = _byteify(json.loads(data_str))
        except ValueError as e:
            print('Could not parse JSON from agent_raspi')
            return False
        
        # Check if the request was completed successfully
        if not data_json['status'] or data_json['status'] != 'OK':
            return False
        
        # Update agent states
        self.agent_temp.active = data_json['output']['agent_temp']['active']
        self.agent_heater.active = data_json['output']['agent_heater']['active']
        self.agent_pycubed.active = data_json['output']['agent_pycubed']['active']
        self.agent_raspi.active = data_json['output']['agent_raspi']['active']
        self.agent_sunsensor.active = data_json['output']['agent_sunsensor']['active']
        self.agent_switch.active = data_json['output']['agent_switch']['active']
        
        # Update IMU
        self.imu.load_json(data_json)
        
        # Update GPS
        self.gps.load_json(data_json)
        
    
        # Update temperature sensors
        self.tempsensor.eps.load_json(data_json)
        self.tempsensor.obc.load_json(data_json)
        self.tempsensor.payload.load_json(data_json)
        self.tempsensor.battery.load_json(data_json)
        self.tempsensor.pycubed.load_json(data_json)
    
        # Update sun sensors
        self.sunsensor.plusx.load_json(data_json)
        self.sunsensor.minusx.load_json(data_json)
        self.sunsensor.plusy.load_json(data_json)
        self.sunsensor.minusy.load_json(data_json)
        self.sunsensor.plusz.load_json(data_json)
        self.sunsensor.minusz.load_json(data_json)
    
        # Update heater
        self.heater.load_json(data_json)
        
    def transmit_file(self, source_file, outgoing_file_name = None):
        """Uses rsync to copy the given file to the BeagleBone, marking it for radio transmission.
                'outgoing_file_name' is the name ONLY of the destination file."""
        
        return self.beaglebone.transmit_file(source_file, outgoing_file_name)
    
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
    

