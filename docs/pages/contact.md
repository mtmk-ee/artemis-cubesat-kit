---
title: Frequently Asked Questions
tags: [faq]
keywords: faq
layout: faq
sidebar: home_sidebar
permalink: /pages/faq.html
folder: /pages
---



<div class="panel-group" id="accordion">
    <div class="panel panel-default">
        <div class="panel-heading">
            <h4 class="panel-title">
                <a class="noCrossRef accordion-toggle" data-toggle="collapse" data-parent="#accordion" href="#collapseOne">After powering my Raspberry Pi I can no longer access it from the BeagleBone</a>
            </h4>
        </div>
        <div id="collapseOne" class="panel-collapse collapse noCrossRef">
            <div class="panel-body">
                Disconnecting power to the Raspberry Pi without shutting it down beforehand can cause the micro SD card containing the operating system to become corrupted. Unfortunately you probably won't be able to recover any data from the card. If you can't even connect to the Raspberry Pi from a desktop machine or laptop, you should try flashing its micro SD card with a fresh image.
            </div>
        </div>
    </div>
    <!-- /.panel -->
    <div class="panel panel-default">
        <div class="panel-heading">
            <h4 class="panel-title">
                <a class="noCrossRef accordion-toggle" data-toggle="collapse" data-parent="#accordion" href="#collapseTwo">Why does the Artemis CubeSat Library take so long to update and copy files?</a>
            </h4>
        </div>
        <div id="collapseTwo" class="panel-collapse collapse noCrossRef">
            <div class="panel-body">
            This is an issue caused by the time it takes to establish a SSH connection between the BeagleBone and Raspberry Pi. To fix this, you can try disabling DNS in the SSH configuration for both the BeagleBone and Raspberry Pi.
            </div>
        </div>
    </div>
    <!-- /.panel -->
    <div class="panel panel-default">
        <div class="panel-heading">
            <h4 class="panel-title">
                <a class="noCrossRef accordion-toggle" data-toggle="collapse" data-parent="#accordion" href="#collapseThree">Some of my SimpleAgent device properties aren't showing up externally, what's happening?</a>
            </h4>
        </div>
        <div id="collapseThree" class="panel-collapse collapse noCrossRef">
            <div class="panel-body">
                Unfortunately there is currently a bug in COSMOS which prevents certain properties from being posted. There's no good way to figure out which ones are broken other than to dig through the COSMOS source code. For now you can try using a different property to store your values. Hopefully this issue gets fixed soon!
            </div>
        </div>
    </div>
    <!-- /.panel -->
    <div class="panel panel-default">
        <div class="panel-heading">
            <h4 class="panel-title">
                <a class="noCrossRef accordion-toggle" data-toggle="collapse" data-parent="#accordion" href="#collapseFour">None of the properties for _one_ of my SimpleAgent devices are showing up externally. What gives?</a>
            </h4>
        </div>
        <div id="collapseFour" class="panel-collapse collapse">
            <div class="panel-body">
                This seems to be a bug in COSMOS, but it's difficult to replicate. For now you can try switching the order you create your devices. If you have only a single device, you can add another device beforehand which goes unused.
            </div>
        </div>
    </div>
    <!-- /.panel -->
    <div class="panel panel-default">
        <div class="panel-heading">
            <h4 class="panel-title">
                <a class="noCrossRef accordion-toggle" data-toggle="collapse" data-parent="#accordion" href="#collapseFive">Why are the power lines on my BeagleBone measuring at strange voltages?</a>
            </h4>
        </div>
        <div id="collapseFive" class="panel-collapse collapse">
            <div class="panel-body">
                Uh oh, that could be bad. Sometimes connecting or disconnecting peripherals while the BeagleBone is powered can damage the internal power lines (we've done this ourselves, unfortunately). If you can't get correct voltages after a while, it could be that your BeagleBone is fried, and you'll need to find a new one.
            </div>
        </div>
    </div>
    <!-- /.panel -->
    <div class="panel panel-default">
        <div class="panel-heading">
            <h4 class="panel-title">
                <a class="noCrossRef accordion-toggle" data-toggle="collapse" data-parent="#accordion" href="#collapseSix">Why am I getting an error before the virtual machine ever boots?</a>
            </h4>
        </div>
        <div id="collapseSix" class="panel-collapse collapse">
            <div class="panel-body">
                Hi
            </div>
        </div>
    </div>
    <!-- /.panel -->
    <div class="panel panel-default">
        <div class="panel-heading">
            <h4 class="panel-title">
                <a class="noCrossRef accordion-toggle" data-toggle="collapse" data-parent="#accordion" href="#collapseSeven">Why can't I auto-resize my virtual machine display?</a>
            </h4>
        </div>
        <div id="collapseSeven" class="panel-collapse collapse">
            <div class="panel-body">
                You'll need to install [[VirtualBox Guest Additions|Setting up the Development Environment#Install Guest Additions]] into the virtual machine
            </div>
        </div>
    </div>
    <!-- /.panel -->
</div>