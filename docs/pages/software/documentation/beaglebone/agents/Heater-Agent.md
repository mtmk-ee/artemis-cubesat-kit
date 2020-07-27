---
title: Heater Agent
tags: [software]
keywords: software
layout: page
#last_updated: July 3, 2016
sidebar: software_sidebar
permalink: /pages/software/documentation/beaglebone/agents/heater-agent.html
folder: /software/documentation/agents/
---

## Quick Details
|   Property   |      Value     |
| ------------ | -------------- |
| Program Name | `agent_heater` |
| Agent Name   | `heater`       |
| Source File  | [agent_heater.cpp]({{ site.gh_repo }}/blob/master/BeagleBone/source/agents/agent_heater.cpp) |
| Hardware     | Heater |
|              | Power Distribution Unit |

## Description
The heater agent (AKA `agent_heater`) is an agent that handles the heater device. Should any temperatures fall too low, the heater agent will enable the heater. Later when the temperatures readings are sufficient, the heater agent will disable the heater. For the specific temperatures different events occur at, see the [temperature events table](#temperature_events)

The physical heater device is controlled using requests issued to the [switch agent](/pages/software/documentation/beaglebone/agents/switch-agent.html) which, in turn, enables or disables the switched line for the heater.

All temperature values are retrieved from the [temperature sensor agent](/pages/software/documentation/beaglebone/agents/temperature-sensor-agent.html).

## Requests

### Heater Control
Usage: `heater [on | off]`

Aliases: `state`, `set`, `get`

#### Description
This request either enables, disables, or displays information for the heater device.

The first argument is optional. If `on` is given, the agent will turn on the heater. If `off` is given, the agent will turn off the heater. If this argument is not supplied, the status of the heater will be returned.

#### Examples
To turn on the heater:

```bash
$ agent cubesat heater heater on
```

To turn off the heater:

```bash
$ agent cubesat heater heater off
```

To view the status of the heater:

```bash
$ agent cubesat heater heater
```

## Temperature Events
Below is a table showing the temperatures various events occur at.

| Temperature (°C) |     Event      |
| ---------------- | -------------- |
|     10 (TODO)    | Heater Enable  |
|     20 (TODO)    | Heater Disable |


## COSMOS Properties
Below is a table showing the devices and COSMOS names for properties used.

<table>
    <thead>
        <tr>
            <th>Device</th>
            <th>Property</th>
            <th>COSMOS name</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td rowspan=4>Heater</td>
            <td>UTC Timestamp</td>
            <td>device_htr_utc_000</td>
        </tr>
        <tr>
            <td>Enabled</td>
            <td>device_htr_enabled_000</td>
        </tr>
        <tr>
            <td>Voltage</td>
            <td>device_htr_volt_000</td>
        </tr>
    </tbody>
</table>


## Block Diagram
Below is a software diagram demonstrating the behavior of the heater agent.

<img src="/resources/diagrams/agent_heater_diagram.png" width="700px">