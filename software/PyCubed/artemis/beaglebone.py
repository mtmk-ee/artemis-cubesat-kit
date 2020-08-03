# CircuitPython modules
import board
import busio

# Standard modules
from datetime import datetime
import time
from pycubed import cubesat as pycubed_cubesat

# Internal modules
import power
import imu
import gps
import temp
import switch
import radio


class BeagleBoneMessageParser:
    def __init__(self, callback):
        self.callback = callback


class BeagleBoneMessageHandler:
    def __init__(self, callback, num_args):
        self.callback = callback
        self.num_args = num_args
        

def parser_pkt(uart, pycubed):
    # Read packet length
    packet_length_str = ''
    while uart.in_waiting() > 0:
        next_char = uart.read(1).decode()
        if next_char == ',':
            break
        else:
            packet_length_str += next_char
    
    # Convert packet length string into an integer
    packet_length = int(packet_length_str)
    
    # Read packet data
    packet_data = uart.read(packet_length)
    
    # Read checksum
    uart.read(1) # Throw out comma
    read_checksum = int(uart.readline().decode(), base=16)
    
    # Make sure the checksum is correct
    # TODO
    
    # Transmit the packet
    radio.radio.send(packet_data)
    
    return True


def handler_bst(args, pycubed):
    pycubed.startup_flag = pycubed.startup_flag or (args[0] == "y")
    pycubed.handoff_flag = pycubed.handoff_flag or (args[1] == "y")
    pycubed.kill_radio_flag = pycubed.kill_radio_flag or (args[2] == "y")


class BeagleBone:
    
    def __init__(self):
        # Set up the UART connection to the BeagleBone
        self.uart = busio.UART(board.TX, board.RX, baudrate=9600)
        self.startup_flag = False
        self.handoff_flag = False
        self.kill_radio_flag = False
        self.handlers = {}
        self.parsers = {}
        
        # Add parsers
        self.add_message_parser('PKT', parser_pkt)
        
        # Add handlers
        self.add_message_handler('BST', handler_bst, 3)

    def update(self):
        # Reset flags
        self.startup_flag = False
        self.handoff_flag = False
        self.kill_radio_flag = False
    
        # Clear the UART line by reading messages
        while self.uart.in_waiting() != 0:
            self.__read_message()
        
        # Check if we need to kill the radio
        if self.kill_radio_flag:
            radio.radio.kill()
            self.kill_radio_flag = False

        # Send sensor data
        self.send_imu_data()
        self.send_gps_data()
        self.send_temp_data()
        self.send_power_data()
        
    def wait_for_startup(self):
        while not self.startup_flag:
        
            # Enable the BeagleBone
            switch.switch.enable("beaglebone")
        
            # Store the time the BeagleBone switch was enabled
            startup_time = time.perf_counter()
        
            # Wait for the BeagleBone to issue the startup flag
            while not self.startup_flag and (time.perf_counter() - startup_time) < 25:
                self.update()
                time.sleep(1)
        
            # Exit if the BeagleBone booted successfully
            if self.startup_flag:
                break
        
            # At this point the BeagleBone did not boot.
            # Disable the BeagleBone
            switch.switch.disable("beaglebone")
        
            # Wait a few seconds
            time.sleep(5)
    
    def add_message_handler(self, name, callback, num_args):
        """Adds a message handler which expects the given number of arguments"""
        self.handlers[name] = BeagleBoneMessageHandler(callback, num_args)
    
    def add_message_parser(self, name, callback):
        """Adds a message parser"""
        self.parsers[name] = BeagleBoneMessageParser(callback)
        
    def __read_message(self):
        """Reads a message"""
        
        # Skip to the syncword
        while self.uart.in_waiting() > 0 and self.uart.read(1)[0] != '$':
            pass
        
        if self.uart.in_waiting() == 0:
            return False
        
        # Read the message type string and throw out the trailing comma
        message_type_str = self.uart.read(4)[0:3].decode()
        
        # Check if there is a message handler
        if message_type_str in self.handlers:
            # Read the entire message
            message = self.uart.readline().decode()
            
            # Strip out the checksum
            checksum = int(message[-2:], 16)
            message = message[0:-3]
            
            # Split the message into an array
            arguments = message.split(",")
            
            # Make sure the list length is correct
            handler = self.handlers[message_type_str]
            if len(arguments) != handler.num_args:
                return False
            
            # Call the handler function
            return self.handlers[message_type_str].callback(arguments, self)
        elif message_type_str in self.parsers:
            return self.parsers[message_type_str].callback(self.uart, self)
        else:
            print("Message type '" + message_type_str + "' is not supported")
            return False
        
    def send_message(self, message_type_str, arguments):
        
        # Add the syncword and type string
        message = '$' + message_type_str + ','
        
        # Add the arguments
        for arg in arguments:
            if arg is str:
                message = message + arg + ','
            else:
                message = message + str(arg) + ','
            
        # Compute the checksum (TODO)
        checksum = 0
        
        # Add the checksum to the message
        message = message + ('{:02x}'.format(checksum)) + '\n'
        
        # Write the message
        self.uart.write(bytearray().extend(message))
        return True
    
    def shutdown(self):
        self.send_message('PST', ['y', 'n', 'n'])
    
    def send_imu_data(self):
        # Create timestamp
        timestamp = datetime.utcnow().strftime("%Y%m%d%H%M%S")
        
        # Get IMU readings
        accel = imu.GetAccel()
        mag = imu.GetMag()
        omega = imu.GetOmega()
        
        # Send message
        return self.send_message('IMU', [
            timestamp,
            accel.x, accel.y, accel.z,
            mag.x, mag.y, mag.z,
            omega.x, omega.y, omega.z])
    
    def send_gps_data(self):
        # Create timestamp
        timestamp = datetime.utcnow().strftime("%Y%m%d%H%M%S")
        
        # Send message
        return self.send_message('GPS', [
            timestamp,
            'y' if gps.gps.has_fix else 'n',
            gps.gps.latitude, gps.gps.longitude, gps.gps.fix_quality,
            gps.gps.satellites, gps.gps.altitude, gps.gps.speed,
            gps.gps.azimuth, gps.gps.horizontal_dilution
        ])
    
    def send_temp_data(self):
        # Create timestamp
        timestamp = datetime.utcnow().strftime("%Y%m%d%H%M%S")
        
        # Write message
        return self.send_message('TMP', [
            timestamp, temp.get_mainboard_temp(), temp.get_batt_temp()
        ])
    
    def send_power_data(self):
        # Create timestamp
        timestamp = datetime.utcnow().strftime("%Y%m%d%H%M%S")

        return self.send_message('PWR', [
            timestamp, power.power.battery_voltage, power.power.charge_current,
            power.power.system_voltage, power.power.system_current
        ])