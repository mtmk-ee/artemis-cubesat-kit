---
title: Setup Part 3
tags: [software]
keywords: software
layout: page
#last_updated: July 3, 2016
sidebar: home_sidebar
permalink: /pages/tutorials/setup/setup3.html
folder: /pages/tutorials/setup
---
| _Pages in this Tutorial_                                  |
| --------------------------------------------------------- |
| 1. [Unboxing the Hardware](setup1.html)                   |
| 2. [Setting up the Development Environment](setup2.html)  |
| **3. Using COSMOS Web**                                   |
| 4. [Running the Camera Script](setup4.html)               |

<div id="toc"></div>

## What You Need
* The assembled cubesat (included)
* A computer with the Ubuntu development environment installed
* USB Type A to USB Type B mini cable

### 1. Connect to the BeagleBone
Using the USB Type A to USB Type B mini cable, connect the port labeled `USB` on the BeagleBone to your computer. This will power and boot the BeagleBone.

Open a web browser inside of Ubuntu and enter `192.168.6.2` into the URL bar. It may take a little while to connect at first, especially if the BeagleBone has not finished booting.

After a while you should see the Cloud9 IDE appear in your browser.

> **_Note:_** on Windows hosts you may need to _eject_ the BeagleBone device from File Explorer first.

<div align="center">
<img src="https://github.com/mtmk-ee/artemis-cubesat-kit/wiki/resources/tutorials/setup/cloud9_home.png" width=600></img>
</div><br>

The COSMOS agents (flight software programs) running on the BeagleBone should automatically start.

### 2. Start COSMOS Web
In an Ubuntu terminal (_not_ the BeagleBone terminal), enter the following commands:

```bash
sudo systemctl start mongod
~/cosmos/tools/agent_mongo
```

Open another terminal and enter the following commands:

```bash
cd ~/cosmos/source/tools/cosmos-web
npm start
```
Your browser should automatically open to a new page and load the COSMOS Web GUI.

> **_Note:_** if the page doesn't open, try entering `0.0.0.0:5000` in a new browser tab


<div align="center">
<img src="https://github.com/mtmk-ee/artemis-cubesat-kit/wiki/resources/tutorials/setup/cosmos_web_home.png" width=600></img>
</div><br>

Next, click on _Realms_ -> _Artemis_ to switch to the Artemis cubesat view. In the box at the top left you will see a list of agents that are currently running on the BeagleBone.

> **_Note:_** if you don't see any agents other than _agent\_mongo_, then you will need to start them manually by running the following commands in a terminal window in the Cloud9 IDE:
> ```bash
> ./start_agents
> ```

<div align="center">
<img src="https://github.com/mtmk-ee/artemis-cubesat-kit/wiki/resources/tutorials/setup/cosmos_web_home.png" width=600></img>
</div><br>

---------------------------------------------------------------------------------------------------------------------------------------------

**Previous:** [[Setting up the Development Environment]] &emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp; **Next:** [[Running the Camera Script]]

