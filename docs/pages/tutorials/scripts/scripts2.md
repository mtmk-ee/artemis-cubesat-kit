---
title: Viewing the Output
tags: [software]
keywords: software
layout: page
#last_updated: July 3, 2016
sidebar: home_sidebar
permalink: /pages/tutorials/scripts/scripts2.html
---

**Language: Python**


## Introduction <a name="introduction"></a>

Payload scripts running on the Raspberry Pi Zero make use of Python. You should probably have a decent understanding of Python before continuing.

In this tutorial, we'll use the [picamera](https://picamera.readthedocs.io/en/release-1.13/) package to control the [Raspberry Pi Camera Module V2](https://www.raspberrypi.org/documentation/hardware/camera/).

## The Payload Script

### Getting Started <a name="getting_started"></a>
First, you'll want to import the necessary packages:

```python
from artemis import cubesat
from picamera import PiCamera
from time import sleep
```
The first line imports the `cubesat` module, which is used to communicate with the rest of the cubesat. The second line imports the `PiCamera` module, which is used to control the Raspberry Pi Camera. The last line imports the `sleep` function, which we will use later in the main loop.

> **_Note:_** if you are modifying your payload device, then you should import any packages needed to control it.

### Initializing the Payload Device <a name="initializing_the_payload_device"></a>

Next, we'll want to initialize the payload device. For the Raspberry Pi Camera it's very straightforward:

```python
camera = PiCamera()
```

You can also configure a bunch of camera settings if you would like. For details on using and configuring the camera using PiCamera, you can visit their [API documentation](https://picamera.readthedocs.io/en/release-1.13/api_camera.html) on this.

### The Main Loop <a name="the_main_loop"></a>
Inside your payload script you will need an infinite `while` loop. Inside this loop you can perform all of the repetitive tasks needed to handle operations with your payload. Additionally, inside the loop you'll need to call the `cubesat`'s update function to make sure it's kept up to date with the latest sensor and agent information.

A simple main loop for the Raspberry Pi Camera might look like this:

```python
while True:
    # Update the cubesat
    cubesat.update()

    # Perform any other tasks here...

    # Wait for some amount of time (in seconds)
    sleep(5)
```

### Putting it all Together <a name="putting_it_all_together"></a>

Here is a basic payload script that captures images using the Raspberry Pi Camera and copies them to the BeagleBone:

```python
# 1. Import necessary modules
from artemis import cubesat
from picamera import PiCamera
from time import sleep

# 2. Initialize the camera
camera = PiCamera()

# (this is the main loop)
while True:
    # 3. Update the cubesat (important!)
    cubesat.update()

    # 4. Pick the destination file on the Raspberry Pi
    image_file = '/home/pi/payload_files/image_%d.jpeg' % image_count
    image_count += 1
    
    # 5. Capture the image to the file we picked
    camera.capture(image_file, 'jpeg')
    
    # 6. Send the image file to the BeagleBone
    cubesat.beaglebone.copy_to(image_file)
    
    # 7. Wait for a little
    sleep(5)
```

Here's an explanation of what is happening at each step:

1. We first import all the necessary modules, as previously explained.
2. We initialize our payload device. In this example we use `picamera` to help us out.
3. We call the vital `cubesat.update` function
4. We pick a file path to capture the image to. Here we've decided to place all our images in a folder called `payload_files` in the home directory, with names `image_XXX.jpeg`, where `XXX` refers to the image number determined by the variable `image_count`.
5. We capture the image to the file path picked in step 2, using a `jpeg` image format. You can check out the picamera API to see which file formats are available.
6. We send our newly-captured image file to the BeagleBone. Unless you provide a destination path, all files will end up in the `/home/debian/raspi/incoming` folder on the BeagleBone with the same file name as the source file.
7. We tell the program to wait for 5 seconds before continuing, using Python's built-in `sleep` function. Without this, the program would quickly cycle through the main loop and would eat up a bunch of memory on the Raspberry Pi and BeagleBone with all those image files.

> **_Note:_** since steps 3 through 7 occur in the main loop, they'll keep running until the program terminates.

## Related Pages <a name="related_pages"></a>
* [[Artemis CubeSat Library]]
