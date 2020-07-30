import subprocess
import os

BEAGLEBONE_OUTGOING_FOLDER = '/home/debian/raspi/outgoing/'
BEAGLEBONE_INCOMING_FOLDER = '/home/debian/raspi/incoming/'
BEAGLEBONE_RADIO_OUTGOING_FOLDER = '/home/debian/cosmos/nodes/cubesat/outgoing/' # For agent_file
BEAGLEBONE_RADIO_INCOMING_FOLDER = '/home/debian/cosmos/nodes/cubesat/incoming/' # For agent_file
BEAGLEBONE_HOST_NAME = 'debian@beaglebone.local'


class BeagleBone:
    """Class for controlling the BeagleBone"""
    
    def __init__(self):
        self._outgoing_folder = BEAGLEBONE_OUTGOING_FOLDER
        self._incoming_folder = BEAGLEBONE_INCOMING_FOLDER
        pass

    def update(self):
        pass

    @staticmethod
    def copy_to(source_file, destination_file=None):
        """Uses rsync to copy the given file to the BeagleBone. If no destination file
        is given, the default folder /home/debian/raspi/incoming will be used, and the
        file name will be the same as the source."""

        # Determine the destination file if none is provided
        if destination_file is None:
            file_name = os.path.basename(source_file)
            destination_file = BEAGLEBONE_INCOMING_FOLDER + file_name

        # Call 'rsync' to copy the file to the BeagleBone
        return subprocess.call(["rsync", "-auv", source_file, BEAGLEBONE_HOST_NAME + ':' + destination_file])

    @staticmethod
    def copy_from(source_file, destination_file):
        """Uses rsync to copy the given file from the BeagleBone."""
        
        # Call 'rsync' to copy the file from the BeagleBone
        return subprocess.call(["rsync", "-auv", BEAGLEBONE_HOST_NAME + ':' + destination_file, source_file])
    
    @staticmethod
    def transmit_file(source_file, outgoing_file_name = None):
        """Uses rsync to copy the given file to the BeagleBone, marking it for radio transmission.
        'outgoing_file_name' is the name ONLY of the destination file."""
        
        # Determine the destination file name if none is provided
        if outgoing_file_name is None:
            outgoing_file_name = os.path.basename(source_file)
        
        return subprocess.call(["rsync", "-auv", source_file, BEAGLEBONE_HOST_NAME + ':' + BEAGLEBONE_RADIO_OUTGOING_FOLDER + outgoing_file_name])
        
    
    @staticmethod
    def system(sys_call):
        """Performs on a system call on the BeagleBone"""
        return subprocess.check_output("ssh %s %s" % (BEAGLEBONE_HOST_NAME, sys_call), shell=True)


