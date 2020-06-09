/********************************************************************
* Copyright (C) 2015 by Interstel Technologies, Inc.
*   and Hawaii Space Flight Laboratory.
*
* This file is part of the COSMOS/core that is the central
* module for COSMOS. For more information on COSMOS go to
* <http://cosmos-project.com>
*
* The COSMOS/core software is licenced under the
* GNU Lesser General Public License (LGPL) version 3 licence.
*
* You should have received a copy of the
* GNU Lesser General Public License
* If not, go to <http://www.gnu.org/licenses/>
*
* COSMOS/core is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3 of
* the License, or (at your option) any later version.
*
* COSMOS/core is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* Refer to the "licences" folder for further information on the
* condititons and terms to use this software.
********************************************************************/

// TODO: change include paths so that we can make reference to cosmos using a full path
// example
// #include <cosmos/core/agent/agentclass.h>

#include "agent/agentclass.h"
#include "support/configCosmos.h"
#include "agent/agentclass.h"
#include "device/serial/serialclass.h"
#include "device/OPT3001.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace cubesat;

// Some nasty global variables
Agent *agent;
ofstream file;
string nodename = "node-cubesat";
string agentname = "imu";

using SunSensorDevice = OPT3001;


struct SunSensor {
	SunSensorDevice *device;
	string name;
	int index;
};

vector<SunSensor> sun_sensors;



void init_agent();
void init_sensors();
void run_agent();

void read_imu();
void read_sun_sensors();
void update_attitude();




int main(int argc, char** argv) {
    cout << "Agent ADCS" << endl;
	
	

    init_agent();
    init_sensors();

    run_agent();


    return 0;
}

/**
 * @brief init_agent Sets up the agent. Prints a message and exits with code 1 if an error occurs.
 */
void init_agent() {
    // Create the agent
    agent = new Agent(nodename, agentname);

    if ( agent->cinfo == nullptr || !agent->running() ) {
        printf("Failed to open [%s:%s]\n", nodename.c_str(), agentname.c_str());
        exit (1);
    }


    // Create the state of health string
    string soh = "{device_imu_mag_000, device_imu_bdot_000, device_imu_accel_000, device_imu_theta_000, "
                 "device_imu_omega_000, device_imu_theta_000, device_imu_utc_000}";

    // set the soh string
    agent->set_sohstring(soh);
    
	run_agent();
}

/**
 * @brief run_agent Runs the main loop for this agent.
 */
void run_agent() {
	
	init_sensors();

    // Start executing the agent
    while ( agent->running() ) {

        // Update sensor readings
        read_imu();
		read_sun_sensors();
        

        // Sleep for a second
        COSMOS_SLEEP(1);
    }

}

/**
 * @brief init_sensors Sets up the sensors for reading
 */
void init_sensors() {
	
	const int NUM_SUN_SENSORS = 6;
	
	// Reserve space for sun sensors
	sun_sensors.reserve(6);
	for (int i = 0; i < NUM_SUN_SENSORS; ++i) {
		sun_sensors.push_back(SunSensor());
		sun_sensors[i].index = i;
	}
	
	
	// Set up sun sensors
	sun_sensors[0].device = new SunSensorDevice();
	sun_sensors[0].name = "plus_x";
	
	sun_sensors[1].device = new SunSensorDevice();
	sun_sensors[1].name = "minus_x";
	
	sun_sensors[2].device = new SunSensorDevice();
	sun_sensors[2].name = "plus_y";
	
	sun_sensors[3].device = new SunSensorDevice();
	sun_sensors[3].name = "minus_y";
	
	sun_sensors[4].device = new SunSensorDevice();
	sun_sensors[4].name = "plus_z";
	
	sun_sensors[5].device = new SunSensorDevice();
	sun_sensors[5].name = "minus_z";
	
	// TODO: initialize devices and verify they are functioning
}

/**
 * @brief update_temps Updates the temperature sensor device specs with their latest readings.
 */
void read_imu() {
    cout << "Updating attitude readings... ";
    
    // TODO: get readings from PyCubed board
    rvector mag, bdot, accel, omega;
    
    
    // Calculate these from sensor readings
    quaternion theta;
    rvector alpha;
    
    agent->cinfo->devspec.imu[0]->mag = mag;
    agent->cinfo->devspec.imu[0]->bdot = bdot;
    agent->cinfo->devspec.imu[0]->accel = accel;
    agent->cinfo->devspec.imu[0]->theta = theta;
    agent->cinfo->devspec.imu[0]->omega = omega;
    agent->cinfo->devspec.imu[0]->theta = theta;
    agent->cinfo->devspec.imu[0]->utc = currentmjd();
    
    cout << "done." << endl;
}


void read_sun_sensors() {
	
	// TODO: add sun sensor device to COSMOS namespace?
	
	for (SunSensor &sensor : sun_sensors) {
		
	}
	
	
}


