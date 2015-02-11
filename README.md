v360
====
This module allows you to remote control the VSN Mobil V.360° Camera.

Requirements
------------
While remote controlling the camera via wifi should work cross-plattform, turning the device off an on will wok on Linux systems only due to the gatttool dependency. For full functionality the following software is required: 
* python3
* pexpect

### Installing dependencies (Ubuntu)
Open a terminal and execute the following commands:

	$ sudo apt-get install python3-pip 
	$ sudo pip3 install pexpect

Usage
-----
### Install and find camera:
Open a terminal and execute the following commands (Ubuntu):

    $ git clone https://github.com/martinruenz/v360.git
    $ cd v360/examples
    $ sudo ./find_cameras.py

### Start video:
Open a terminal and execute the following commands (Ubuntu):

    $ ./run_camera.py