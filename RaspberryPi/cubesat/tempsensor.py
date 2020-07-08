


class TempSensor:

    def __init__(self, sensor_name):
        self._sensor_name = sensor_name
        self._temp = 0
        self._utc = 0
        pass

    def parse_soh_string(self, soh_str):
        """Parses the SOH string for relevant sensor data."""
        pass
    
    @property
    def name(self):
        return self._sensor_name
    
    @property
    def utc(self):
        return self._utc
        
    @property
    def temp(self):
        return self._temp

    @temp.setter
    def temp(self, value):
        self._temp = value




