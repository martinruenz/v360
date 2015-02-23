V360 Remote Control
===================
This module allows you to remote control the VSN Mobil V.360° Camera.
The remote control is written in Python but the interface is also available for C++. After taking a look at the examples folder the available methods should be self-explanatory.

Although the remote control is already working, be aware that the code is still in an early stage.

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