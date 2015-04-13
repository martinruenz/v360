#!/usr/bin/env python3
import os, sys, time
sys.path.insert(0, os.path.abspath(".."))
import v360

pin = '0000' # use your camera-pin here
btmac = 'B4:99:4C:4B:CE:1C' # use cam.findCameras() to obtain this values or call 'hcitool lescan'

cam = v360.Remote()

print('Try to: Connect, take picture, disconnect')
cam.verbose = False
if cam.connect(pin):
	print('Enter photo-mode', cam.setMode('photo'))
	print('Take picture', cam.takePicture())
	time.sleep(5)
	cam.disconnect()
else:
	print('Could not connect to camera.')	