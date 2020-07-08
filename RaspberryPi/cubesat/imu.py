from collections import namedtuple

Vec3 = namedtuple('Vec3', 'x', 'y', 'z')


class IMU:

    def __init__(self):
        self._magnetic = (0, 0, 0)
        self._acceleration = (0, 0, 0)
        self._gyro = (0, 0, 0)
        self._utc = 0
        pass

    def parse_soh_string(self, soh_str):
        """Parses the PyCubed SOH string for relevant sensor data."""
        pass
    
    @property
    def utc(self):
        return self.utc

    @property
    def magnetic(self):
        return self._magnetic

    @magnetic.setter
    def magnetic(self, value):
        self._magnetic = value

    @property
    def acceleration(self):
        return self._acceleration

    @acceleration.setter
    def acceleration(self, value):
        self._acceleration = value

    @property
    def gyro(self):
        return self._gyro

    @gyro.setter
    def gyro(self, value):
        self._gyro = value


