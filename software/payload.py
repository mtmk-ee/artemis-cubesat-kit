from time import sleep
from picamera import PiCamera 

class raspberrypi():
    def __init__():
        camera = PiCamera()
        camera.resolution(1024, 768)
        camera.start_preview()
        sleep(2)

    #take picture in jpg and resize
    def take_pic():
        camera.capture('foo.jpg', resize=(320,240))

    #makes 60 second long video in h264
    def take_vid():
        camera.start_recording('video.h264')
        camera.wait_recording(60)
        camera.stop_recording()

    #timelapse with picture taken every five seconds    
    def timelapse():
        for filename in camera.capture_continuous('img{counter:03d}.jpg'):
            sleep(5)
