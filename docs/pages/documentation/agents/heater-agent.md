---
title: Heater Agent
layout: agent
permalink: /pages/documentation/agents/heater-agent.html

sidebar: home_sidebar
tags: [software, heater]
keywords: software

agent_name: agent_heater

---


## Description
The heater agent (AKA `agent_heater`) is an agent that handles the heater device. Should any temperatures fall too low, the heater agent will enable the heater. Later when the temperatures readings are sufficient, the heater agent will disable the heater. For the specific temperatures different events occur at, see the [temperature events table](#temperature-events)

The physical heater device is controlled using requests issued to the [switch agent][agent_switch] which, in turn, enables or disables the switched line for the heater.

All temperature values are retrieved from the [temperature sensor agent][agent_temp].

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
|     10           | Heater Enable  |
|     20           | Heater Disable |



[agent_switch]: {{site.baseurl}}/pages/documentation/agents/switch-agent.html
[agent_temp]: {{site.baseurl}}/pages/documentation/agents/temperature-sensor-agent.html