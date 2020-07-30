---
title: "Getting Started Part 2 - Development Environment"
layout: series
permalink: /pages/tutorials/setup/setup2.html

tags: [software]
keywords: software
sidebar: home_sidebar
toc: false


series:
  prev: /pages/tutorials/setup/setup1.html
  next: /pages/tutorials/setup/setup3.html
---

## What You Need

* A computer running Windows, macOS, or a Linux distribution
  * As the tools used tend to eat up RAM and processing power, it is recommended you have a decently powerful computer
* An internet connection
* A USB flash drive with the development environment (included)

## 1. Install VirtualBox

First you'll need to install [VirtualBox](https://www.virtualbox.org/wiki/Downloads) on your computer. This is a neat program that allows you to run _entire operating systems_ (called virtual machines) running atop your actual one. We've decided to use VirtualBox for maximum portability, as it runs on most operating systems.

## 2. Download 7-Zip
Since the disk image is compressed, you'll need to install [7-Zip](https://www.7-zip.org/download.html) to decompress it.

## 3. Set up the Virtual Machine

{% include note.html content='Having problems with this step? Visit the <a href="faq.html#virtualbox">FAQ page</a>!' %}

Now that VirtualBox is installed, you can add the virtual machine containing the development environment.

### 3.1 Obtain the Disk Image
You can choose either to copy the disk image from the flash drive to your computer, or to download the disk image from the [Artemis repository](https://github.com/mtmk-ee/artemis-cubesat-kit/) (recommended if your flash drive image is out of date).

### 3.2 Unzip the Disk Image

### 3.3 Add the Virtual Machine to VirtualBox

First, click on the new button to add a new virtual machine.

<div align="center">
<img src="https://github.com/mtmk-ee/artemis-cubesat-kit/wiki/resources/tutorials/setup/virtualbox_home_new.png" width=600></img>
</div><br>

You'll want to name the operating system something descriptive (e.g. Artemis Development Environment).
Change the _Type_ field to _Linux_, and the _Version_ field to _Ubuntu (64-bit)_ as shown below, and click _Next_.
<div align="center">
<img src="https://github.com/mtmk-ee/artemis-cubesat-kit/wiki/resources/tutorials/setup/virtualbox_new_vm.png" width=400></img>
</div><br>

Now you have the option of setting how much of your computer's RAM will be allocated for the virtual machine. Setting this value too low means that the virtual machine will run slowly, while setting it too high could slow down could cause other issues. Using a value near the right end of the green zone is a good idea. After choosing your RAM value, click _Next_.

<div align="center">
<img src="https://github.com/mtmk-ee/artemis-cubesat-kit/wiki/resources/tutorials/setup/virtualbox_memory_size.png" width=400></img>
</div><br>

Here you will be able to add the development environment disk image. Click _Use an existing virtual hard disk file_, then click the folder icon, then the _Add_ button in the new window, and browse for the disk image (the `.vdi` file) you previously obtained. Click _Next_.

<div align="center">
<img src="https://github.com/mtmk-ee/artemis-cubesat-kit/wiki/resources/tutorials/setup/virtualbox_add_drive.png" width=400></img>
</div><br>

Now that the virtual machine is installed, we should probably change a couple more settings. Select your new virtual machine on the left, and click on the _Settings_ button at the top.

<div align="center">
<img src="https://github.com/mtmk-ee/artemis-cubesat-kit/wiki/resources/tutorials/setup/virtualbox_home_settings.png" width=600></img>
</div><br>

In the System -> Processor tab you can choose to allow the virtual machine to use multiple physical processors.
In the Display -> Screen tab you should enable the _Enable 3D Acceleration_ option to allow rendering to be offloaded to your GPU.

Now you can go back to the VirtualBox home view and click on the _Start_ button to start the virtual machine.


#### 3.4 Logging In
After clicking the start button in the previous step, you should see the virtual machine booting.

Once Ubuntu has booted, you should be met with a login screen. The default username is _osboxes_, and the default password is _osboxes.org_

{% include warning.html content='For security, you may want to consider changing the password.' %}

#### 3.5 Install Guest Additions

Inside of Ubuntu, open up a new terminal by pressing <kbd>Control</kbd> + <kbd>Alt</kbd> + <kbd>T</kbd>. If this doesn't work, you can also click on the apps menu (the grid of dots on the side of the screen), type in _Terminal_, and click on the application to run it.

Type in the following commands, separated by pressing the <kbd>Enter</kbd> key.

```bash
sudo apt update
sudo apt install build-essential dkms linux-headers-$(uname -r)
```

{% include note.html content='Until you finishing installing guest additions, copying and pasting between your host machine and virtual machine will not work.' %}

Next, click on the _Devices_ menu at the top of the virtual machine, and click _Insert Guest Additions CD Image_ at the bottom of the list, and click the _Run_ button in the dialog that pops up. The program may take a few minutes to run, so feel free to go make some coffee.

Once the program is finished installing guest additions, go ahead and reboot the virtual machine by clicking the power icon at the top right of the screen, click the power icon in the menu, and clicking _Restart_.

Once Ubuntu boots up, you can log back in and verify that guest additions was installed successfully by entering the following command into a terminal:

```bash
lsmod | grep vboxguest
```

If the installation was successful, you see output which looks like this (the numbers will probably be different):

```bash
vboxguest             303104  2 vboxsf
```
<br>

Now we can enable some handy functionality for the guest machine:

* To enable copying and pasting between the host and guest, click _Devices_->_Shared Clipboard_->_Bidirectional_ at the top of the screen.
* To enable drag and drop between the host and guest, click _Devices_->_Drag and Drop_->_Bidirectional_.
* To enable automatic resizing of the guest display, click _View_->_Auto-resize Guest Display_.


{% include tip.html content='When pasting text into a terminal window, the keyboard shortcut <kbd>Control</kbd> + <kbd>Shift</kbd> + <kbd>V</kbd> is used. For copying text from a terminal window, the keyboard shortcut <kbd>Control</kbd> + <kbd>Shift</kbd> + <kbd>C</kbd> is used.' %}

{% include note.html content='There is currently an issue with VirtualBox on Windows machines which prevents you from dragging and dropping "complex" objects such as zip archives and directories. To get around this, you can set up a shared folder.' %}


## Further Reading
* [Artemis Development Environment]({{site.folder_docs_other}}/development-environment.html)
