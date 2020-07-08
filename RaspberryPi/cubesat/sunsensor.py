


class SunSensor:

    def __init__(self, sensor_name):
        self._sensor_name = sensor_name
        self._lux = 0
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
    def lux(self):
        return self._lux

    @lux.setter
    def lux(self, value):
        self._lux = value




