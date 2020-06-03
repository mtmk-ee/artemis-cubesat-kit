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
#include <iostream>
#include <fstream>

using namespace std;

// Some nasty global variables
Agent *agent;
ofstream file;
string nodename = "node-cubesat";
string agentname = "payload";

void init_agent();
void init_payload();
void run_agent();






int main(int argc, char** argv) {
    cout << "Agent Thermal" << endl;


    init_agent();
    init_payload();
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
    
}

/**
 * @brief run_agent Runs the main loop for this agent.
 */
void run_agent() {

    // Start executing the agent
    while ( agent->running() ) {
        
        

        // Sleep for a second
        COSMOS_SLEEP(1);
    }

}



void init_payload() {
    // TODO
}


