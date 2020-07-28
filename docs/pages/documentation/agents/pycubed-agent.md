---
title: PyCubed Agent
layout: agent
permalink: /pages/documentation/agents/pycubed-agent.html

sidebar: home_sidebar
tags: [software, pycubed]
keywords: software

agent_name: agent_pycubed
---

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
