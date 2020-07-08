from cubesat import CubeSat
from time import sleep


# Create a new CubeSat object to expose cubesat functionality
cubesat = CubeSat()

# Main loop
while True:
    #update cubesat
    cubesat.update()

    print('='*40)


    heater = cubesat.agent_heater
    pycubed = cubesat.agent_pycubed
    raspi = cubesat.agent_raspi
    sunsesnor = cubesat.agent_sunsensor
    switch = cubesat.agent_switch
    temp = cubesat.agent_temp

    print('Agent Heater')
    for heaters in heater.NUM_HEATERS:
        print('\t| Sensor "%s"' % heaters.name)
        print('\t|\t| Timestamp: %d' % heaters.utc)
        print('\t|\t| Temperature: %d' % heaters.temp)
    print('\t| Handle Temps: %s' % heater.handle_temps()
    print('\t| Enable: %s' % heater.set_heater_state(True))
    print('\t| Disable: %s' % heater.set_heater_state(False))
    

    print('Agent Pycbued')
    print('\t| SOH: %s' % pycubed.set_soh.soh)
    print('\t| IMU: %d' % pycubed.handle_pycubed.pycubed_info.imu.info)
    print('\t| GPS: %d' % pycubed.handle_pycubed.pycubed_info.gps.info)
    print('\t| Batt: %d' % pycubed.handle_pycubed.pycubed_info.batt.info)

    print('Agent Raspi')
    print('\t| SOH: %s' % raspi.set_soh.soh)
    #incoming files

    print('Agent Sunsensor')
    print('\t| SOH: %s' % sunsensor.set_soh.soh)
    for sunsensor in sunsensor.SUNSENSOR_COUNT:
        print('\t| Sensor "%s"' % sunsensor.name)
        print('\t|\t| Timestamp: %d' % sunsensor.utc)
        print('\t|\t| Lux: %d' % sunsensor.lux)
    print('\t| String of names: %s' % sunsesnor.name_list)

    print('Agent Switch')
    print('\t| SOH: %s' % switch.set_soh.soh)
    for switch in switch.SWITCH_COUNT:
        print('\t| Switch "%s"' % switch.switch_name)
        print('\t|\t| Timestamp: %s' % switch.switch_status)


    print('Agent Temp')
    print('\t| SOH: %s' % temp.set_soh.soh)
    #destroy sensors
