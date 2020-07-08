import subprocess
import json

BEAGLEBONE_HOST_NAME = 'debian@beaglebone.local'


class Agent:

    def __init__(self, name, node='cubesat'):
        self.name = name
        self.node = node
        
    @staticmethod
    def dosomething():
        print("hi")

    def request(self, request_str):
        """Sends an agent request to the BeagleBone and returns the result string."""
        return subprocess.check_output("ssh %s agent %s %s %s" % (BEAGLEBONE_HOST_NAME, self.node, self.name, request_str), shell=True)
    
    def getvalues(self, value_names):
        """Retrieves values of the given names from the agent's SOH."""
        
        # Create the request string
        json_str = "{"
        for value in value_names:
            json_str += '"'
            json_str += value
            json_str += '", '
        
        json_str = json_str[0:-2]
        json_str += "}"
        
        # Send the request
        response = self.request('getvalue ' + json_str)
        
        # Parse as JSON
        return json.loads(response)
        
    @property
    def soh(self):
        
        """Returns the latest SOH for the given agent"""
        pass



