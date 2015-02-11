#!/usr/bin/env python3
import os, sys, time
sys.path.insert(0, os.path.abspath(".."))
import v360
cam = v360.Remote()

cam.findCameras(doprint=True)