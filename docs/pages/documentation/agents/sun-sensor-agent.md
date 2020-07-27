---
title: Sun Sensor Agent
tags: [software]
keywords: software
layout: page
#last_updated: July 3, 2016
sidebar: home_sidebar
permalink: /pages/documentation/agents/sun-sensor-agent.html
---

## Quick Details
|   Property   |      Value     |
| ------------ | -------------- |
| Program Name | `agent_sun`    |
| Agent Name   | `sun`          |
| Source File  | [agent_sun.cpp]({{ site.gh_repo }}/blob/master/BeagleBone/source/agents/agent_sun.cpp) |
| Hardware     | [OPT3001 I<sup>2</sup>C Sun Sensor](https://www.ti.com/lit/ds/symlink/opt3001.pdf) |

## Description
The sun sensor agent (aka `agent_sun`) is an agent that handles operations with the sun sensors. This includes initialization, configuration, and reading values from the sun sensors. Nothing is done with the values obtained from the sun sensors, but they are available for use elsewhere, such as in an ADCS agent (not included).

There are six sun sensors, with one located on each faces of the cubesat.

## Requests

### Sensor Status
Usage: `sensor <device name>`

Aliases: N/A

#### Description
This request returns the status of a given sensor.

The argument `device name` corresponds to the [name of the sun sensor](#sensor_names).

#### Examples
To display the status of the sun sensor on the +X (plus x) side of the structure:

```bash
$ agent cubesat sun sensor ss_plusx
```

### List Available Sensors
Usage: `list`

Aliases: N/A

#### Description

This request lists all available sun sensors.

#### Example

To view all available sun sensors:

```bash
$ agent cubesat sun list
```

## Sensor Names
Below is a table showing which sensor names correspond to which physical devices.

| Sensor Name    | ID Number | Physical Device | PC104 Header Pin |
| -------------- | --------- | --------------- | ---------------- |
| `ss_plusx`     |     0     | +X Sun Sensor   |       TODO       |
| `ss_minusx`    |     1     | -X Sun Sensor   |       TODO       |
| `ss_plusy`     |     2     | +Y Sun Sensor   |       TODO       |
| `ss_minusy`    |     3     | -Y Sun Sensor   |       TODO       |
| `ss_plusz`     |     4     | +Z Sun Sensor   |       TODO       |
| `ss_minusz`    |     5     | -Z Sun Sensor   |       TODO       |



## COSMOS Properties
Below is a table showing the devices and COSMOS names for properties used by the sun sensor agent.

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
            <td rowspan=3>+X Sun Sensor</td>
            <td>UTC Timestamp</td>
            <td>device_ssen_utc_000</td>
            <td>double</td>
        </tr>
        <tr>
            <td>Illumination (lux) </td>
            <td>device_ssen_temp_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Voltage (V) </td>
            <td>device_ssen_volt_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td rowspan=3>-X Sun Sensor</td>
            <td>UTC Timestamp</td>
            <td>device_ssen_utc_001</td>
            <td>double</td>
        </tr>
        <tr>
            <td>Illumination (lux) </td>
            <td>device_ssen_temp_001</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Voltage (V) </td>
            <td>device_ssen_volt_001</td>
            <td>float</td>
        </tr>
        <tr>
            <td rowspan=3>+Y Sun Sensor</td>
            <td>UTC Timestamp</td>
            <td>device_ssen_utc_002</td>
            <td>double</td>
        </tr>
        <tr>
            <td>Illumination (lux) </td>
            <td>device_ssen_temp_002</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Voltage (V) </td>
            <td>device_ssen_volt_002</td>
            <td>float</td>
        </tr>
        <tr>
            <td rowspan=3>-Y Sun Sensor</td>
            <td>UTC Timestamp</td>
            <td>device_ssen_utc_003</td>
            <td>double</td>
        </tr>
        <tr>
            <td>Illumination (lux) </td>
            <td>device_ssen_temp_003</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Voltage (V) </td>
            <td>device_ssen_volt_003</td>
            <td>float</td>
        </tr>
        <tr>
            <td rowspan=3>+Z Sun Sensor</td>
            <td>UTC Timestamp</td>
            <td>device_ssen_utc_004</td>
            <td>double</td>
        </tr>
        <tr>
            <td>Illumination (lux) </td>
            <td>device_ssen_temp_004</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Voltage (V) </td>
            <td>device_ssen_volt_004</td>
            <td>float</td>
        </tr>
        <tr>
            <td rowspan=3>-Z Sun Sensor</td>
            <td>UTC Timestamp</td>
            <td>device_ssen_utc_005</td>
            <td>double</td>
        </tr>
        <tr>
            <td>Illumination (lux) </td>
            <td>device_ssen_temp_005</td>
            <td>float</td>
        </tr>
        <tr>
            <td>Voltage (V) </td>
            <td>device_ssen_volt_005</td>
            <td>float</td>
        </tr>
    </tbody>
</table>

## Block Diagram
Below is a software diagram demonstrating the behavior of the sun sensor agent.

<img src="/resources/diagrams/agent_sun_diagram.png" width="600px">