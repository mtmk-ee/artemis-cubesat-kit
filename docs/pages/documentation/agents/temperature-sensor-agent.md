---
title: Temperature Sensor Agent
tags: [software]
keywords: software
layout: page
#last_updated: July 3, 2016
sidebar: home_sidebar
permalink: /pages/documentation/agents/temperature-sensor-agent.html
---


## Quick Details
|   Property   |      Value     |
| ------------ | -------------- |
| Program Name | `agent_temp`   |
| Agent Name   | `temp` |
| Source File  | [agent_temp.cpp]({{ site.gh_repo }}/blob/master/BeagleBone/source/agents/agent_temp.cpp) |
| Hardware     | [ADT7311 Temperature Sensor](https://www.analog.com/media/en/technical-documentation/data-sheets/ADT7311.pdf) |
|              | [BMX160 IMU](https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BMX160-DS000.pdf) |

## Description
The temperature sensor agent (aka `agent_temp`) is an agent that handles operations with the temperature sensors.

There are two different ways that the temperature sensors are hooked up. Some sensors are connected to the BeagleBone SPI headers through the PC104 headers, and the others must be accessed through the [PyCubed agent](pycubed-agent.html). The temperature sensor agent handles temperature collection for both cases.

## Requests

### Sensor Status
Usage: `sensor <device name>`

Aliases: N/A

#### Description
This request returns the status of a given sensor.

The argument `device name` corresponds to the [name of the temperature sensor](#sensor_names).

#### Examples
To display the status of the battery temperature sensor:

```bash
$ agent cubesat temp sensor temp_battery
```

### List Available Sensors
Usage: `list`

Aliases: N/A

#### Description

This request lists all available temperature sensors

#### Example

To view all available temperature sensors:

```bash
$ agent cubesat temp list
```

## Sensor Names
Below is a table showing which sensor names correspond to which physical devices.

| Sensor Name    | ID Number |             Physical Device               | PC104 Header Pin |
| -------------- | --------- | ----------------------------------------- | ---------------- |
| `temp_eps`     |     0     | EPS Temperature Sensor (ADT7311)          |       TODO       |
| `temp_obc`     |     1     | BeagleBone Temperature Sensor (ADT7311)   |       TODO       |
| `temp_raspi`   |     2     | Raspberry Pi Temperature Sensor (ADT7311) |       TODO       |
| `temp_battery` |     3     | Battery Temperature Sensor (ADT7311)      |       N/A        |
| `temp_pycubed` |     4     | PyCubed Temperature Sensor (BMX160)       |       N/A        |


## COSMOS Properties
Below is a table showing the devices and COSMOS names for properties used by the temperature sensor agent.

<table>
    <thead>
        <tr>
            <th>Device</th>
            <th>Property</th>
            <th>COSMOS Name</th>
            <th>Storage Type</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td rowspan=3>EPS Temperature Sensor</td>
            <td>UTC Timestamp</td>
            <td>device_tsen_utc_000</td>
            <td>double</td>
        </tr>
        <tr>
            <td>Temperature (K) </td>
            <td>device_tsen_temp_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Voltage (V) </td>
            <td>device_tsen_volt_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td rowspan=3>BeagleBone Temperature Sensor</td>
            <td>UTC Timestamp</td>
            <td>device_tsen_utc_001</td>
            <td>double</td>
        </tr>
        <tr>
            <td>Temperature (K) </td>
            <td>device_tsen_temp_001</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Voltage (V) </td>
            <td>device_tsen_volt_001</td>
            <td>float</td>
        </tr>
        <tr>
            <td rowspan=3>Raspberry Pi Temperature Sensor</td>
            <td>UTC Timestamp</td>
            <td>device_tsen_utc_002</td>
            <td>double</td>
        </tr>
        <tr>
            <td>Temperature (K) </td>
            <td>device_tsen_temp_002</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Voltage (V) </td>
            <td>device_tsen_volt_002</td>
            <td>float</td>
        </tr>
        <tr>
            <td rowspan=3>Battery Temperature Sensor</td>
            <td>UTC Timestamp</td>
            <td>device_tsen_utc_003</td>
            <td>double</td>
        </tr>
        <tr>
            <td>Temperature (K) </td>
            <td>device_tsen_temp_003</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Voltage (V) </td>
            <td>device_tsen_volt_003</td>
            <td>float</td>
        </tr>
        <tr>
            <td rowspan=3>PyCubed Temperature Sensor</td>
            <td>UTC Timestamp</td>
            <td>device_tsen_utc_004</td>
            <td>double</td>
        </tr>
        <tr>
            <td>Temperature (K) </td>
            <td>device_tsen_temp_004</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Voltage (V) </td>
            <td>device_tsen_volt_004</td>
            <td>float</td>
        </tr>
    </tbody>
</table>

## Block Diagram
Below is a software diagram demonstrating the behavior of the temperature sensor agent.

<img src="/resources/diagrams/agent_temp_diagram.png" width="700px">