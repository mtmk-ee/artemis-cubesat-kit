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

#define SLEEP_TIME 1

using namespace std;

string nodename = "node-cubesat";
string agentname = "thermal";
Agent *agent;
Agent *agent_pycubed;
ofstream file;

struct Message {
    string data;
    double utc;
};

vector<Message> send_queue;
vector<Message> received_queue;

void init_agent();
void run_agent();

void read_messages();
void send_messages();
void exec_messages();
void exec_message(Message *msg);

int32_t request_send(char *request, char* response, Agent *agent);
//int32_t request_read(char *request, char* response, Agent *agent);

int main(int argc, char** argv) {
    cout << "Agent COMMS" << endl;
    
    init_agent();
    run_agent();

    return 0;
}

void init_agent() {
    // Create the agent
    agent = new Agent(nodename, agentname);

    if ( agent->cinfo == nullptr || !agent->running() ) {
        printf("Failed to open [%s:%s]\n", nodename.c_str(), agentname.c_str());
        exit (1);
    }


    // Create the state of health string
    string soh = "{}";


    // set the soh string
    agent->set_sohstring(soh);
    
    // Add request callbacks
    int status;
    
    if ( (status = agent->add_request("send", request_send)) )
        exit(status);
    
    //if ( (status = agent->add_request("read", request_read)) )
    //    exit(status);
}

void run_agent() {

    // Start executing the agent
    while ( agent->running() ) {
        
        read_messages();
        send_messages();
        exec_messages();

        // Sleep for a second
        COSMOS_SLEEP(SLEEP_TIME);
    }
}

/**
 * @brief read_messages Pulls messages from PyCubed
 */
void read_messages() {
    
    // 1. Send request to PyCubed to pull messages
    // 2. Add messages to queue
    
    
}

/**
 * @brief send_messages Flushes send queue to PyCubed
 */
void send_messages() {
    
    // TODO: Flush queue to PyCubed RF transmitter
}

/**
 * @brief exec_messages Executes received messages
 */
void exec_messages() {
    
    for (auto it = received_queue.begin(); it != received_queue.end(); ++it) {
        
        // Execute the message
        exec_message(&*it);
        
        // Clear from queue
        received_queue.erase(it);
    }
    
}

/**
 * @brief exec_message Executes the command received from a particular message
 * @param msg The message
 */
void exec_message(Message *msg) {
    // TODO
}


/**
 * @brief request_transmit Handles a request to transmit data
 * @return 
 */
int32_t request_transmit(char *request, char* response, Agent *agent) {
    //
}

