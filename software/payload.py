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

    #infinite loop picture every 1 second
    def take_pic():
        while True:
            newpic = camera.capture('foo[%d].jpg' % i, resize=(320,240))
            pictures.append(newpic)
            sleep(1)

    #makes 60 second long video in h264
    def take_vid():
        camera.start_recording('video.h264')
        camera.wait_recording(60) #waits for 60 seconds
        camera.stop_recording()

    #timelapse with picture taken every five seconds    
    def timelapse():
        for filename in camera.capture_continuous('img{counter:03d}.jpg'):
            sleep(5)

    #Shut down camera
    def off():
        close()
