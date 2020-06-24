from time import sleep
from picamera import PiCamera 

class raspberrypi():
    def __init__():
        camera = PiCamera()
        camera.resolution(1024, 768)
        camera.start_preview()
        sleep(2)
        i = 0
        pictures = []

    #infinite loop picture every specified amount of time
    def take_pic(time):
        while True:
            newpic = camera.capture('foo{counter:03d}.jpg', resize=(320,240))
            pictures.append(newpic)
            sleep(time)

    #makes video in h264 for certain amount of time
    def take_vid(time)):
        camera.start_recording('video.h264')
        camera.wait_recording(60) #waits for 60 seconds
        camera.stop_recording(time)

    #timelapse with picture taken once per allotted amount fo time   
    def timelapse(time):
        for filename in camera.capture_continuous('img{counter:03d}.jpg'):
            sleep(time)

    #stops all functions and releases all resources
    def off():
        close()
