---
title: Raspberry Pi Agent
layout: agent
permalink: /pages/documentation/agents/raspberry-pi-agent.html

sidebar: home_sidebar
tags: [software, raspberrypi]
keywords: software

agent_name: agent_raspi

# "Quick Description" info

---

## Description
The Raspberry Pi agent (aka `agent_raspi`) is an agent that handles communication with the Raspberry Pi, including startup, shutdown, and data transfer.

## Requests

### Secure Shell Commanding
Usage: `ssh <command>`

Aliases: `command`

#### Description
This request executes a command on the Raspberry Pi through a SSH connection.

The argument `command` is a shell command to run on the Raspberry Pi.

#### Examples
To list the contents of the home directory (`/home/pi/`) of the Raspberry Pi, we can use the Linux `ls` command:

```bash
$ agent cubesat raspi ssh ls /home/pi/
```

### Check Connection
Usage: `ping`

Aliases: `is_up`

#### Description

This request attempts to ping the Raspberry Pi to determine whether it is up or down. Internally this uses the Linux `ping` command to check if the Raspberry Pi is detected on the local network.

If the Raspberry Pi can be reached, the request will return `"UP`". Otherwise, the request will return `"DOWN"`.

#### Example

To check if the Raspberry Pi can be reached:

```bash
$ agent cubesat raspi ping
```

### Shut Down Raspberry Pi
Usage: `shutdown_raspi`

Aliases: `end`

#### Description

This request attempts to shut down the Raspberry Pi. Internally this executes `sudo shutdown now` on the Raspberry Pi.

> **_Note:_** the request name `shutdown` is not used, as this is reserved by COSMOS for a different purpose.

#### Example

To shut down the Raspberry Pi:

```bash
$ agent cubesat raspi shutdown_raspi
```

### Get Agent Data
Usage: `get_data`

Aliases: `agent_data`

#### Description

This request returns all collected data from other agents as a JSON-formatted string. This request is primarily used by the Artemis CubeSat Library on the Raspberry Pi to fetch data.

#### Example

To view all available agent data:

```bash
$ agent cubesat raspi get_data
```

## Available Agent Data
Below is a table showing which properties are collected from various other agents. This data can be viewed through the [`get_data`](#get_agent_data) request.

(TODO)

<table>
    <thead>
        <tr>
            <th>Agent Program</th>
            <th>Device</th>
            <th>JSON Key</th>
            <th>Description</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td rowspan=6>agent_temp</td>
            <td>N/A</td>
            <td>agent_temp.active</td>
            <td>Whether or not the agent can be reached</td>
        </tr>
        <tr>
            <td>EPS Temperature Sensor</td>
            <td>agent_temp.temp_eps</td>
            <td>Temperature of the EPS</td>
        </tr>
        <tr>
            <td>BeagleBone Temperature Sensor</td>
            <td>agent_temp.temp_obc</td>
            <td>Temperature of the BeagleBone</td>
        </tr>
        <tr>
            <td>Raspberry Pi Temperature Sensor</td>
            <td>agent_temp.temp_raspi</td>
            <td>Temperature of the Raspberry Pi</td>
        </tr>
        <tr>
            <td>Battery Temperature Sensor</td>
            <td>agent_temp.temp_battery</td>
            <td>Temperature of the batteries</td>
        </tr>
        <tr>
            <td>PyCubed Temperature Sensor</td>
            <td>agent_temp.temp_pycubed</td>
            <td>Temperature of the PyCubed</td>
        </tr>
        <tr>
            <td rowspan=7>agent_sun</td>
            <td>N/A</td>
            <td>agent_sun.active</td>
            <td>Whether or not the agent can be reached</td>
        </tr>
        <tr>
            <td>+X Sun Sensor</td>
            <td>agent_sun.ss_plusx</td>
            <td>Lux reading from the +X sun sensor</td>
        </tr>
        <tr>
            <td>-X Sun Sensor</td>
            <td>agent_sun.ss_minusx</td>
            <td>Lux reading from the -X sun sensor</td>
        </tr>
        <tr>
            <td>+Y Sun Sensor</td>
            <td>agent_sun.ss_plusy</td>
            <td>Lux reading from the +Y sun sensor</td>
        </tr>
        <tr>
            <td>-Y Sun Sensor</td>
            <td>agent_sun.ss_minusy</td>
            <td>Lux reading from the -Y sun sensor</td>
        </tr>
        <tr>
            <td>+Z Sun Sensor</td>
            <td>agent_sun.ss_plusz</td>
            <td>Lux reading from the +Z sun sensor</td>
        </tr>
        <tr>
            <td>-Z Sun Sensor</td>
            <td>agent_sun.ss_minusz</td>
            <td>Lux reading from the -Z sun sensor</td>
        </tr>
        <tr>
            <td rowspan=7>agent_pycubed</td>
            <td>N/A</td>
            <td>agent_pycubed.active</td>
            <td>Whether or not the agent can be reached</td>
        </tr>
        <tr>
            <td rowspan=3>IMU</td>
            <td>Acceleration</td>
            <td>agent_pycubed.imu.accel</td>
            <td>A vector containing acceleration readings</td>
        </tr>
        <tr>
            <td>Magnetic Field</td>
            <td>agent_pycubed.imu.mag</td>
            <td>A vector containing magnetic field readings</td>
        </tr>
        <tr>
            <td>Angular Acceleration</td>
            <td>agent_pycubed.imu.gyro</td>
            <td>A vector containing angular acceleration readings</td>
        </tr>
    </tbody>
</table>