#!/usr/bin/env python3
import os, sys, time
sys.path.insert(0, os.path.abspath(".."))
import v360

pin = '0000' # use your camera-pin here
btmac = 'B4:99:4C:4B:CE:1C' # use cam.findCameras() to obtain this values or call 'hcitool lescan'

cam = v360.Remote()
if cam.getName() is None:
	print('Not connected to camera wifi, try booting camera...')
	cam.turnOn(btmac) 
	time.sleep(25)

print('Camera name:', cam.getName())
if cam.connect(pin):
	print('Connected! Play video for 30 seconds and disconnect.')
	print('Battery %s%%' % cam.getBattery())
	print('HDMI', cam.getHdmiConnected())
	print('Powered', cam.getPowerConnected())
	print('Start video', cam.startVideo())
	time.sleep(30)
	print('Stop video', cam.stopVideo())
	cam.disconnect()

	print('Now reconnect (just for demonstration), play a video with sepia effect for another 30 seconds and turn off.')
	cam.verbose = False
	if cam.connect(pin):
		print('Battery %s%%' % cam.getBattery())
		print('HDMI', cam.getHdmiConnected())
		print('Powered', cam.getPowerConnected())
		cam.setVideoEffect('sepia')
		print('Start video', cam.startVideo())
		time.sleep(30)
		print('Stop video', cam.stopVideo())
		cam.setVideoEffect('none')
		cam.turnOff()
	else:
		print('Could not reconnect to camera.')	

else:
	print('Could not connect to camera. Make sure you (automatically) connected to the camera wifi.')

