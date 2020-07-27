---
title: PyCubed Agent
tags: [software]
keywords: software
layout: page
#last_updated: July 3, 2016
sidebar: home_sidebar
permalink: /pages/documentation/agents/pycubed-agent.html
---

## Quick Details
|   Property   |      Value      |
| ------------ | --------------- |
| Program Name | `agent_pycubed` |
| Agent Name   | `pycubed`       |
| Source File  | [agent_pycubed.cpp]({{ site.gh_repo }}/blob/master/BeagleBone/source/agents/agent_pycubed.cpp) |
| Hardware     | PyCubed Mainboard |

## Description
The PyCubed agent (AKA `agent_pycubed`) is an agent that handles all communication between the BeagleBone and PyCubed, making use of the [PyCubed class](/pages/software/documentation/beaglebone/utility/pycubed-class.html).

## Requests

### Check Connection
Usage: `is_up`

Aliases: N/A

#### Description
This request checks if the PyCubed can be reached. If the PyCubed is connected properly, the request will return `"UP"`. Otherwise, the request will return `"DOWN"`.

#### Examples
To check if the PyCubed can be reached:

```bash
$ agent cubesat pycubed is_up
```

### Send a Message
Usage: `send_message message_str`

Aliases: `send`

#### Description
This request sends a given message to the PyCubed.

The argument `message_str` is a message formatted to the [PyCubed communication protocol](/pages/software/documentation/pycubed/communication-protocol.html) requirements. The leading `$` and checksum should not be included.

#### Examples
To send a successful startup message:

```bash
$ agent cubesat pycubed send_message BST,y,n,n
```

### Get IMU Data
Usage: `imu`

Aliases: N/A

#### Description
This request returns the latest PyCubed IMU information as a JSON-formatted string.

#### Examples
To get the latest IMU information:

```bash
$ agent cubesat pycubed imu
```

### Get GPS Data
Usage: `gps`

Aliases: N/A

#### Description
This request returns the latest PyCubed GPS information as a JSON-formatted string.

#### Examples
To get the latest GPS information:

```bash
$ agent cubesat pycubed gps
```

### Get Power Data
Usage: `power`

Aliases: N/A

#### Description
This request returns the latest PyCubed power information as a JSON-formatted string.

#### Examples
To get the latest power information:

```bash
$ agent cubesat pycubed power
```

### Get Temperature Data
Usage: `temperature`

Aliases: `temp`

#### Description
This request returns the latest PyCubed temperature information as a JSON-formatted string.

#### Examples
To get the latest temperature information:

```bash
$ agent cubesat pycubed temperature
```

### Kill the Radio
Usage: `killradio yes`

Aliases: N/A

#### Description
This request sends a message to the PyCubed to kill all future radio transmission.

The argument `yes` is required, and is meant to prevent accidental usage of the request.

> **_Note:_** **You should not call this request** unless the radio is transmitting harmful noise, or if you are performing tests.

#### Examples
To kill the radio _(be careful!)_:

```bash
$ agent cubesat pycubed killradio yes
```

## Communication Protocol
For details visit [[PyCubed Communication Protocol]].

## COSMOS Properties
Below is a table showing the devices and COSMOS names for properties used.

<table>
    <thead>
        <tr>
            <th>Device</th>
            <th>Property</th>
            <th>COSMOS name</th>
            <th>Storage Type</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td rowspan=7>PyCubed CPU</td>
            <td>UTC Timestamp</td>
            <td>device_cpu_utc_000</td>
            <td>double or Time</td>
        </tr>
        <tr>
            <td>Temperature (K)</td>
            <td>device_cpu_temp_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Voltage (V)</td>
            <td>device_cpu_volt_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Current (A)</td>
            <td>device_cpu_amp_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Memory Used (GiB)</td>
            <td>device_cpu_gib_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Maximum Memory (GiB)</td>
            <td>device_cpu_gib_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Up Time (s)</td>
            <td>device_cpu_uptime_000</td>
            <td>int</td>
        </tr>
        <tr>
            <td rowspan=8>Battery</td>
            <td>UTC Timestamp</td>
            <td>device_batt_utc_000</td>
            <td>double or Time</td>
        </tr>
        <tr>
            <td>Temperature (K)</td>
            <td>device_batt_temp_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Voltage (V)</td>
            <td>device_batt_volt_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Current (A)</td>
            <td>device_batt_amp_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Capacity (Ah)</td>
            <td>device_batt_cap_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Charge (Ah)</td>
            <td>device_batt_charge_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Percentage (%)</td>
            <td>device_batt_percentage_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Efficiency (%)</td>
            <td>device_batt_efficiency_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td rowspan=5>IMU</td>
            <td>UTC Timestamp</td>
            <td>device_imu_utc_000</td>
            <td>double or Time</td>
        </tr>
        <tr>
            <td>Temperature (K)</td>
            <td>device_imu_temp_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Linear Acceleration (m/s<sup>2</sup>)</td>
            <td>device_imu_accel_000</td>
            <td>Vec3</td>
        </tr>
        <tr>
            <td>Angular Velocity (°/s)</td>
            <td>device_imu_omega_000</td>
            <td>Vec3</td>
        </tr>
        <tr>
            <td>Magnetic Field (G)</td>
            <td>device_imu_mag_000</td>
            <td>Vec3</td>
        </tr>
        <tr>
            <td rowspan=4>GPS</td>
            <td>UTC Timestamp</td>
            <td>device_gps_utc_000</td>
            <td>double or Time</td>
        </tr>
        <tr>
            <td>Latitude (°) / Longitude (°) / Altitude (m)</td>
            <td>device_gps_geods_000</td>
            <td>Location</td>
        </tr>
        <tr>
            <td>Velocity (m/s)</td>
            <td>device_gps_geocv_000</td>
            <td>Vec3</td>
        </tr>
        <tr>
            <td>Satellites Used</td>
            <td>device_gps_sats_used_000</td>
            <td>int</td>
        </tr>
        <tr>
            <td rowspan=4>Node</td>
            <td>Power Use (W)</td>
            <td>node_powuse</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Power Generation (W)</td>
            <td>node_powgen</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Battery Capacity (Ah)</td>
            <td>node_battcap</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Battery Charge (Ah)</td>
            <td>node_battlev</td>
            <td>float</td>
        </tr>
    </tbody>
</table>


## Block Diagram
Below is a software diagram demonstrating the behavior of the PyCubed agent.

<img src="/resources/diagrams/agent_pycubed_diagram.png" width="900px">