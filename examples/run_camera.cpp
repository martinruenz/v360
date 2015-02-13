#include <string>
#include "../v360.hpp"

using namespace std;

const string pin = "0000"; // use your camera-pin here
const string btmac = "B4:99:4C:4B:CE:1C"; // use findCameras() to obtain this values or call 'hcitool lescan'

int main(){

	v360::Remote cam = v360::Remote();
	v360::Variant name = cam.getName();
	if(name.isSet()) cout << name << endl;
	else cout << "Not connected to camera wifi, try booting camera..." << endl;

	/*
	print(
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

	print('Now reconnect, play a video with sepia effect for another 30 seconds and turn off.')
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

*/

	return 0;
}