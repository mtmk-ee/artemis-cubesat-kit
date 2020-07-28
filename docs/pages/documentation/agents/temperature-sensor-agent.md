---
title: Temperature Sensor Agent
layout: agent
permalink: /pages/documentation/agents/temperature-sensor-agent.html

sidebar: home_sidebar
tags: [software]
keywords: software

agent_name: agent_temp
---

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
