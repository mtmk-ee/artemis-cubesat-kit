---
title: SimpleAgent
tags: [software]
keywords: software
layout: page
#last_updated: July 3, 2016
sidebar: home_sidebar
permalink: /pages/documentation/utility-classes/simpleagent.html
---

Note: this wiki page provides documentation for the SimpleAgent class. For the tutorial, see the [SimpleAgent tutorial](/pages/software/tutorials/beaglebone/utility/simpleagent-tutorial.html).

## Quick Details
|   Property   |      Value     |
| ------------ | -------------- |
| Source Files  | [SimpleAgent.h]({{ site.gh_repo }}/blob/master/BeagleBone/include/utility/SimpleAgent.h) <br> [SimpleAgent.cpp]({{ site.gh_repo }}/blob/master/BeagleBone/source/utility/SimpleAgent.cpp) |

## Description
The `SimpleAgent` class is a wrapper for existing COSMOS capabilities. It provides a cleaner and more straightforward way of using COSMOS agents.

For details on supported devices, see [[SimpleAgent Devices]].

## Member Functions

Below is a list of the most important member functions. For a complete documentation, visit [[Doxygen Documentation]].

| Function | Description |
| -------- | ----------- |
| `SimpleAgent(const std::string &name, const std::string &node = "cubesat")` | Constructs a new SimpleAgent with the given name, running on the given node. Will throw an exception if another agent has already been created in the same program |
| `static SimpleAgent* GetInstance()` | Returns the instance of the SimpleAgent, or `nullptr` if no agent was previously created |
| `Agent* GetComplexAgent()` | Returns the regular COSMOS agent |
| `bool IsRunning()` | Checks if the agent is running |
| `void Shutdown()`  | Shuts down the agent |
| `void SetLoopPeriod(double period)` | Sets the interval in seconds between iterations of the main loop |
| `bool StartLoop()` | Marks the start of a main loop iteration. Returns `true` if the loop should execute. |
| `void DebugPrint()` | Prints a table containing all devices, requests, and posted properties |
| `T* NewDevice<T>(const std::string &name)` | Creates a new device with the given type `T`, and with the given name. Returns a pointer to the new device. Do not delete this pointer! For supported device types, see [[SimpleAgent Devices]] |
| `T* GetDevice<T>(const std::string &name)` | Retrieves a previously-added device with the given name and type `T`. If the device does not exist, `nullptr` is returned. For supported device types, see [[SimpleAgent Devices]] |
| `bool AddRequest(const std::string &name, ArgumentedRequest callback, std::string synopsis = "", std::string description = "")` | Adds an argumented request to the agent with the given name and callback, using the given synopsis and detailed description strings. Returns `true` upon success |
| `bool AddRequest(const std::string &name, NonArgumentedRequest callback, std::string synopsis = "", std::string description = "")` | Adds a non-argumented request to the agent with the given name and callback, using the given synopsis and detailed description strings. Returns `true` upon success |
| `bool RequestExists(const std::string &name)` | Returns `true` if a request with the given name has previously been added |