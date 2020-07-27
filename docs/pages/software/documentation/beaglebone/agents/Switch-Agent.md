---
title: Switch Agent
tags: [software]
keywords: software
layout: page
#last_updated: July 3, 2016
sidebar: software_sidebar
permalink: /pages/software/documentation/beaglebone/agents/switch-agent.html
folder: /software/documentation/agents/
---

## Quick Details
|   Property   |      Value     |
| ------------ | -------------- |
| Program Name | `agent_switch` |
| Agent Name   | `switch`       |
| Source File  | [agent_switch.cpp]({{ site.gh_repo }}/blob/master/BeagleBone/source/agents/agent_switch.cpp) |
| Hardware     | Power Distribution Unit |

## Description
The switch agent (aka `agent_switch`) is an agent that provides control to the switched lines in the EPS power distribution unit (PDU). Little is actually done inside of the main loop of this agent, as most functionality is performed through requests.

The PDU switched lines are controlled using several of the BeagleBone's GPIO pins. The switch agent internally either sets a GPIO pin HIGH to enable a switched line, or sets a GPIO pin LOW to disable it.

## Requests

### Switch Line Control
Usage: `switch <device name> [on | off]`

Aliases: `state`, `set`, `get`

#### Description
This request either enables, disables, or displays information for a given switch.

The argument `device_name` corresponds to the [name of the switch](#pin mapping).

The second argument is optional. If `on` is given, the agent will turn on the given switch. If `off` is given, the agent will turn off the given switch. If this argument is not supplied, the status of the given switch will be returned.

#### Examples
To turn on the temperature sensor switch:

```bash
$ agent cubesat switch switch sw_temp on
```

To turn off the temperature sensor switch:

```bash
$ agent cubesat switch switch sw_temp off
```

To view the status of the temperature sensor switch:

```bash
$ agent cubesat switch switch sw_temp
```

### List Available Switches
Usage: `list`

Aliases: N/A

#### Description

This request lists all available switches.

#### Example

To view all available switches:

```bash
$ agent cubesat switch list
```

## Pin Mapping
Below is a table showing which GPIO pins control which switched lines:

| Switch Name | ID Number |   Physical Device   | GPIO Pin | PC104 Header Pin | PDU Switched Line |
| ----------- | --------- | ------------------- | -------- | ---------------- | ----------------- |
| `sw_heater` |     0     |        Heater       |  TODO    |       TODO       |       TODO        |
| `sw_temp`   |     1     | Temperature Sensors |  TODO    |       TODO       |       TODO        |
| `sw_ss`     |     2     |      Sun Sensors    |  TODO    |       TODO       |       TODO        |


## COSMOS Properties
Below is a table showing the devices and COSMOS names for properties used by the switch agent.

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
            <td rowspan=3 style="text-align:center">Heater Switch</td>
            <td>UTC Timestamp</td>
            <td>device_swch_utc_000</td>
            <td>double</td>
        </tr>
        <tr>
            <td>Enabled</td>
            <td>device_swch_enabled_000</td>
            <td>bool</td>
        </tr>
        <tr>
            <td>Voltage</td>
            <td>device_swch_volt_000</td>
            <td>float</td>
        </tr>
        <tr>
            <td rowspan=3 style="text-align:center">Temperature Sensor Switch</td>
            <td>UTC Timestamp</td>
            <td>device_swch_utc_001</td>
            <td>double</td>
        </tr>
        <tr>
            <td>Enabled</td>
            <td>device_swch_enabled_001</td>
            <td>bool</td>
        </tr>
        <tr>
            <td>Voltage</td>
            <td>device_swch_volt_001</td>
            <td>float</td>
        </tr>
        <tr>
            <td rowspan=3 style="text-align:center">Sun Sensor Switch</td>
            <td>UTC Timestamp</td>
            <td>device_swch_utc_002</td>
            <td>double</td>
        </tr>
        <tr>
            <td>Enabled</td>
            <td>device_swch_enabled_002</td>
            <td>bool</td>
        </tr>
        <tr>
            <td>Voltage</td>
            <td>device_swch_volt_002</td>
            <td>float</td>
        </tr>
    </tbody>
</table>

## Block Diagram
Below is a software diagram demonstrating the behavior of the switch agent.

<img src="/resources/diagrams/agent_switch_diagram.png" width="400px">