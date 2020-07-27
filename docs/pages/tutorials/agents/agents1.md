---
title: COSMOS and QtCreator
tags: [software]
keywords: software
layout: page
#last_updated: July 3, 2016
sidebar: home_sidebar
permalink: /pages/tutorials/agents/agents1.html
folder: /pages/tutorials/agents
---

**Language: C++**

## Prerequisites
* [[Setup Tutorial|Unboxing the Hardware]]

## Introduction
QtCreator is an IDE (integrated development environment) targeted at C/C++ developers, and is included in the [[Artemis Development Environment]] by default.


## Creating a New COSMOS Project
Open up the `~/cosmos/source/projects/` directory. This folder contains all COSMOS projects that you will write.

Duplicate the project folder `simpleagent_template` and rename it as you wish. In this tutorial, we will use the project name `my_project`.

Open up QtCreator and go to _File_ -> _Open File or Project..._, and open the file `~/cosmos/source/projects/my_project/CMakeLists.txt`. This file contains the project configuration.

<div align="center">
<img src="https://github.com/mtmk-ee/artemis-cubesat-kit/wiki/resources/tutorials/qtcreator/open_project_menu.png" width=400></img>
<img src="https://github.com/mtmk-ee/artemis-cubesat-kit/wiki/resources/tutorials/qtcreator/open_project_window.png" width=400></img>
</div><br>

The project `simpleagent_template` will appear in the navigation pane on the left side of the window (you can always rename the project by editing the `CMakeLists.txt` file). Expand the project (double click), then expand the target `my_agent` and open the source file `my_agent.cpp`.

You should now see the `my_agent.cpp` source code in the editor.

## Writing the Agent
For a tutorial on writing agents, visit the [[SimpleAgent tutorial|SimpleAgent Tutorial]]

## Build and Deploying the Agent
To build the program, click the hammer icon at the bottom left of the screen. This will take a while on the first build since QtCreator needs to build the COSMOS Core source, but subsequent builds will be faster. If your program is bug-free, you'll see a green bar at the bottom right of QtCreator when the build is complete.

Next, you'll need to deploy your agent to the BeagleBone. You can copy the build folder to the BeagleBone by running the following terminal command:

```bash
rsync -auv ~/cosmos/source/projects/my_project/build-beaglebone debian@beaglebone.local:/home/debian
```

From Cloud9 on the BeagleBone you should see the build folder appear in the home directory. To run the agent, you can run the following command in a Cloud9 terminal:

```bash
~/build-beaglebone/my_agent
```

You should now see the output from your agent!

> **_Note:_** if you get an error when trying to run the program, run the following command to give the file "execute" permission and enter the password (default is `temppwd`):
> ```bash
> sudo chmod +x ~/build-beaglebone/my_agent
> ```

