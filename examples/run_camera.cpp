#include <string>
#include "../v360.hpp"

#include <iostream>

#ifdef unix
#include <unistd.h>
#endif 

#ifdef _WIN32
#include <windows.h>
void sleep(int s)
{
	Sleep(s * 1000);
}
#endif


using namespace std;

const string pin = "0000"; // use your camera-pin here
const string btmac = "B4:99:4C:4B:CE:1C"; // use findCameras() to obtain this values or call 'hcitool lescan'

int main(){

	v360::Remote cam = v360::Remote();
	v360::Variant name = cam.getName();
	if(name.isSet()) cout << name << endl;
	else {
		cout << "Not connected to camera wifi, try booting camera..." << endl;

#ifdef unix
		cam.turnOn(btmac)
		sleep(25);
#else
		cout << "Your system does not provide gatttool. Hence, turning on the camera via bluetooth LE is not possible." << endl;
#endif 
	}

	name = cam.getName();
	if (name.isSet()) cout << name << endl;

	if (cam.connect(pin)){
		cout << "Connected! Play video for 30 seconds and disconnect." << endl;
		cout << "Battery " << cam.getBattery() << "%" << endl;
		cout << "HDMI " << cam.getHdmiConnected() << endl;
		cout << "Powered " << cam.getPowerConnected() << endl;
		cout << "Start video " << cam.startVideo() << endl;
		sleep(25);
		cout << "Stop video " << cam.stopVideo() << endl;
		cam.disconnect();


		cout << "Now reconnect, play a video with sepia effect for another 30 seconds and turn off." << endl;
		cam.setVerbose(false);
		if (cam.connect(pin)){
			cout << "Battery " << cam.getBattery() << "%" << endl;
			cout << "HDMI " << cam.getHdmiConnected() << endl;
			cout << "Powered " << cam.getPowerConnected() << endl;
			cam.setVideoEffect("sepia");
			cout << "Start video " << cam.startVideo() << endl;
			sleep(30);
			cout << "Stop video " << cam.stopVideo() << endl;
			cam.setVideoEffect("none");
			cam.turnOff();
		}
		else{
			cout << "Could not reconnect to camera." << endl;
		}
	}
	else {
		cout << "Could not connect to camera. Make sure you (automatically) connected to the camera wifi." << endl;
	}

	return 0;
}