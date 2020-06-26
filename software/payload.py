import json
from time import sleep
from picamera import PiCamera 

class raspberrypi():
    def __init__():
        camera = PiCamera()
        camera.resolution(1024, 768)
        camera.start_preview()
        camera.iso(400)
        sleep(2)
        pictures = []
        close = False
        camera = {}

    #infinite loop picture every specified amount of time
    def take_pic():
        while True:
            newpic = camera.capture('foo{counter:03d}.jpg')
            Add_Json(picture, newpic)
            sleep(time)

    #set soh
    def SetSOHString(soh_string):
        soh_string = ('Camera SOH\n\t"State: %d"\n\t"iso: %s"\n\t"resolution: %s, %s"\n\t"interval time: %s"\n',
            % (state, iso, width, height, time))
        cubesat.SetSOHString(soh_string)


    #set resolution, if either are invalid entries, sets to 0
    def set_res(height, width):
        if height is none or height == '' or height <= 0:
            res_width = 1024
            res_height = 768
        
        elif height is none or height == '' or height <= 0:
            res_width = 1024
            res_height = 768            
        
        else:
            res_width = width
            res_height = height
        res = (res_height, res_width)
        camera.resolution(res_width, res_height)
    

    #set time between pictures
    def set_time(interval):
        if interval is none or interval == '' or interval == 0:
            time = 1
        else:
            time = interval


    #set iso, 100, 200, 400, 800, 1600, 3200, 6400 valid entries
    #automatically sets higher if value set between
    def set_iso(iso):
        if iso is none or iso == '' or iso <= 0:
            camera.iso(400)
            iso_value = 400
        
        else:
            camera.iso(iso)
            iso_value = int camer.iso(iso)

    def get_state():
    
    #stops all functions and releases all resources
    def off():
        close = True
        close()
