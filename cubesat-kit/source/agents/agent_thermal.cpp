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
string agentname = "thermal";

enum TempSensorID {
    TEMP_SENSOR_IMU,
    TEMP_SENSOR_BATT,
    TEMP_SENSOR_OBC,
    TEMP_SENSOR_COUNT, // The number of temperature sensors
};

void init_agent();
void init_sensors();
void run_agent();
void update_temps();
bool handle_temp(TempSensorID sensor);
float read_temp(TempSensorID sensor);


int32_t request_heater(char *request, char* response, Agent *agent);
int32_t request_sensor(char *request, char* response, Agent *agent);




int main(int argc, char** argv) {
    cout << "Agent Thermal" << endl;


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
    string soh = "{";

    for (int i = 0; i < TEMP_SENSOR_COUNT; ++i) {
        // Convert device number to string
        std::ostringstream ss;
        ss << std::setw(3) << std::setfill('0') << i;

        // Add to the SOH string
        soh.append("\"device_tsen_temp_" + ss.str() + "\", ");
        soh.append("\"device_tsen_utc_" + ss.str());

        // Add delimiter if necessary
        if ( i != TEMP_SENSOR_COUNT - 1 )
            soh.append(", ");
    }

    soh.append("}");


    // set the soh string
    agent->set_sohstring(soh);
    
    // Add request callbacks
    int status;
    
    if ( (status = agent->add_request("heater", request_heater)) )
        exit (status);
    
    if ( (status = agent->add_request("sensor", request_sensor)) )
        exit (status);
}

/**
 * @brief run_agent Runs the main loop for this agent.
 */
void run_agent() {

    // Start executing the agent
    while ( agent->running() ) {

        // Update sensor readings
        update_temps();
        
        // Check readings
        bool nominal = true;
        
        for (int i = 0; i < TEMP_SENSOR_COUNT; ++i) {
            
            if ( !handle_temp((TempSensorID)i) ) {
                nominal = false;
            }
        }
        
        if ( nominal ) {
            cout << "All temperature sensors reading as nominal." << endl;
        }

        // Sleep for a second
        COSMOS_SLEEP(1);
    }

}

/**
 * @brief init_sensors Sets up the sensors for reading
 */
void init_sensors() {
    // TODO
}

/**
 * @brief update_temps Updates the temperature sensor device specs with their latest readings.
 */
void update_temps() {
    cout << "Updating temperature log... ";
    
    for (int i = 0; i < TEMP_SENSOR_COUNT; ++i) {
        float temp = read_temp((TempSensorID)i);

        agent->cinfo->devspec.tsen[i]->temp = temp;
        agent->cinfo->devspec.tsen[i]->utc = currentmjd();
    }
    
    cout << "done." << endl;
}

/**
 * @brief read_temp Reads the temperature from a specific sensor.
 * @param sensor The ID of the sensor
 * @return The temperature reading in Kelvin
 */
float read_temp(TempSensorID sensor) {

    // TODO: Read sensors
    switch ( sensor ) {
    default:
        break;
    }

    // For now, return a test temperature
    return 276.0f + (float)sensor;
}

/**
 * @brief temp_within_range Checks if a temperature reading is within of the necessary range, and acts if necessary.
 * @param sensor The sensor
 * @return True if the temperature is within the correct range.
 */
bool handle_temp(TempSensorID sensor) {

    // Pull the latest temperature reading
    float temp = agent->cinfo->devspec.tsen[(int)sensor]->temp;

    // TODO: fetch min and max temperatures. Currently using placeholder values.
    float min_temp = 273;
    float max_temp = 323;
    float epsilon = 5; // The margin at which to issue warnings.

    bool within_range = true;

    if ( temp < min_temp ) { // Case: Temperature too low (critical)
        cout << "Critical: Temperature sensor " << (int)sensor << " reads as too low (currently at " <<
                temp << " K, min is " << min_temp << " K)." << endl;
        within_range = false;
    }
    else if ( temp < min_temp + epsilon ) { // Case: Temperature too low (warning)
        cout << "Warning: Temperature sensor " << (int)sensor << " is nearing lower boundary (currently at " <<
                temp << " K, min is " << min_temp << " K)." << endl;
        within_range = false;
    }
    else if ( temp > max_temp ) { // Case: Temperature too high (critical)
        cout << "Critical: Temperature sensor " << (int)sensor << " reads as too high (currently at " <<
                temp << " K, max is " << max_temp << " K)." << endl;
        within_range = false;
    }
    else if ( temp > max_temp - epsilon ) { // Case: Temperature too high (warning)
        cout << "Warning: Temperature sensor " << (int)sensor << " is nearing upper boundary (currently at " <<
                temp << " K, max is " << max_temp << " K)." << endl;
        within_range = false;
    }



    return within_range;
}

/**
 * @brief request_heater Request for enabling/disabling heaters. Usage: heater ### [on | off]
 * @return 
 */
int32_t request_heater(char *request, char* response, Agent *agent) {
    // TODO: Add ability to reference heater by name
    
    int heater_id;
    char heater_state_str[32];
    
    int status = sscanf(request, "%*s %d %s", &heater_id, heater_state_str);
    
    // Find which state the heater needs to be set to (0 = off, 1 = on, 2 = invalid)
    int heater_state = (strcmp(heater_state_str, "on") == 0) ? true : (strcmp(heater_state_str, "off") == 0 ? false : 2);
    
    // Check if the given request is invalid
    if ( status != 2 || heater_state == 2 ) {
        sprintf(response, "Usage: heater ### [on | off]");
        cout << "Invalid request" << endl;
        return 1;
    }
    else if ( heater_state ){
        sprintf(response, "Enabling heater %d", heater_id);
        cout << "Enabling heater " << heater_id << endl;
    }
    else {
        sprintf(response, "Disabling heater %d", heater_id);
        cout << "Disabling heater " << heater_id << endl;
    }
    
    
    return 0;
}

/**
 * @brief request_sensor Request for sensor data. Usage: sensor ### 
 * @return 
 */
int32_t request_sensor(char *request, char* response, Agent *agent) {
    // TODO: Add ability to reference sensor by name
    int sensor_id;
    int heater_id;
    
    int status = sscanf(request, "%*s %d", &heater_id);
    
    
    // Check if the given request is invalid
    if ( status != 1 ) {
        sprintf(response, "Usage: sensor ###");
        return 1;
    }
    else {
        sprintf(response, "%f", agent->cinfo->devspec.tsen[heater_id]->temp);
    }
    
    
    return 0;
}


