import json, ssl, urllib
from urllib.request import urlopen
from urllib.request import Request
import time
import threading
import codecs

class CachedJSON:
	"""Cache data in order to reduce traffic"""
	def __init__(self, name, remote, request, cache_duration=10):

		self.name = name
		self.remote = remote
		self.request = request
		self.cache_duration = cache_duration
		self.last_refresh = time.time() - (cache_duration + 5)
		self.data = None

	def isOutdated(self):
		if time.time() - self.last_refresh < self.cache_duration:
			return False
		return True

	def update(self):
		if self.remote.verbose:
			print('Trying to download', self.name, '...')

		response = self.remote.sendRequest(self.request)

		if response is None:
			print('Downloading', self.name, 'failed!')
			return False

		self.data = json.load(self.remote.u8reader(response))
		self.last_refresh = time.time()

		if self.remote.verbose:
			print('JSON: ', self.data)

		return True

	def readElement(self, key):

		if self.isOutdated():
			if not self.update():
				return None
		elif self.remote.verbose:
				print('Got',self.name,'from cache.')

		if type(key) is str:
			return self.data[key]
		return self.data[key[0]][key[1]]

def callUntilSuccess(function, times=0.1):
	while times is not 0:
		if function():
			return True
		times -= 1
	return False

class Remote: 
	"""Class for remote controlling the v360 camera from VSN"""
	def __init__(self):

		self.verbose = True
		self.timeout = 7
		self.login_tries = 3 # try to login 'n' times (0 means until success)
		self.reconnect = True
		self.buffer_duration = 10
		self.camera_ip = '192.168.0.1'
		self.token = None
		self.pin = None
		self.last_http_status = None
		self.camera_info = CachedJSON("camera info", self, 'http://'+self.camera_ip+'/status/CAMINF')
		self.camera_status = None # cached json, after connecting
		self.camera_settings = None # cached json, after connecting
		self.camera_wfconf = None # wifi enabled? TODO
		self.video_settings = None # cached json, after connecting
		self.photo_settings = None # cached json, after connecting
		self.stop_heartbeat = False
		self.u8reader = codecs.getreader("utf-8")

	def requireToken(self):
		if self.token is None:
			print('This operation is only available when connected to the camera.')
			return False
		return True

	def findCameras(self, doprint=False):
		"""Find cameras via low energy blootooth.
		   This method requires gattlib and root privileg.
		"""
		from gattlib import DiscoveryService

		try:
			service = DiscoveryService("hci0")
			devices = service.discover(2)
		except RuntimeError:
			print('Discovering devices failed. Are you root and do you have a bluetooth device?')
			return None

		if self.verbose or doprint:
			print("Found cameras:")
			for address, name in devices.items():
				print("name: {}, address: {}".format(name, address))

		return devices # TODO: This returns all bluetooth LE devices

	def turnOn(self, bluetooth_mac, interface=None, repeat=3):
		""" Turn on camera with mac 'bluetooth_mac' via bluetooth LE.
			This method requires pexpect and gatttool.
			
			Returns False if camera is not turned on afterwards, otherwise True
		"""
		
		if repeat > 0:
			return callUntilSuccess(lambda: self.turnOn(bluetooth_mac, interface, 0), int(repeat))

		try:
			import pexpect
		except ImportError:
			print('Turning on the camera via bluetooth does require the "pexpect" package. (pip3 install pexpect)')
			return False

		if interface is None:
			interface = ''
		else:
			interface = ' -i ' + interface
		import sys
		gattcmd = 'gatttool -b ' + bluetooth_mac + interface + ' -I'
		gatttool = pexpect.spawnu(gattcmd)
		
		if self.verbose:
			print('calling:', gattcmd)
			gatttool.logfile = sys.stdout

		try:
			gatttool.sendline('connect')
			gatttool.expect('.*\[CON.*',timeout=4)
			time.sleep(0.4)
			gatttool.sendline('char-write-req 0x0030 303030300000000000000000')
			gatttool.expect('.*success.*',timeout=3)
			time.sleep(0.4)
			gatttool.sendline('char-write-req 0x0034 0100')
			gatttool.expect('.*success.*',timeout=3)
			time.sleep(0.4)
			gatttool.sendline('char-write-req 0x002d 0001')
			gatttool.expect('.*success.*',timeout=3)
		except pexpect.TIMEOUT:	
			print('Could not connect to the bluetooth device.')
			return False 

		return True

	def turnOff(self):
		if not self.requireToken():
			return False

		response = self.sendJSON('https://'+self.camera_ip+'/status/POWSTA', {"powsta": False, "token": self.token})

		if response is None:
			print('Turning off failed.')
			return False

		self.disconnect()

		return True

	def sendRequest(self, request):
		try:
			response = urlopen(request, timeout = self.timeout)
			self.last_http_status = response.getcode()

			if self.verbose and not "HTBEAT" in response.geturl():
				print('Response headers: ', response.info(), 'Code: ', self.last_http_status)



		#except URLError, e:
		except IOError as e:
			#print('Communication with', request.url, 'failed.')
			if hasattr(e, 'code'): # HTTPError
				print('Http error, code: ', e.code)
				if e.code == 401: # unauthorized => disconnected
					self.token = None
				self.last_http_status = e.code
			else:
				self.last_http_status = None
			if hasattr(e, 'reason'): # URLError
				print("Connection failed, reason: ", e.reason)
			else:
				print("Connection failed: ", e)

			return None
		return response

	def sendJSON(self, address, data):
		encoded_data = json.dumps(data).encode('utf8')
		request = Request(address, encoded_data, {'Content-Type': 'application/json'})
		return self.sendRequest(request)

	def initCachedValues(self):
		self.camera_status = CachedJSON("camera status", self, 'https://'+self.camera_ip+'/status/BASSTA?token='+self.token)
		self.camera_settings = CachedJSON("camera settings", self, 'https://'+self.camera_ip+'/settings/general?token='+self.token)
		self.video_settings = CachedJSON("video settings", self, 'https://'+self.camera_ip+'/settings/video?token='+self.token)
		self.photo_settings = CachedJSON("photo settings", self, 'https://'+self.camera_ip+'/settings/still?token='+self.token)

	def login(self, pin, repeat = 3):

		if repeat > 0:
			return callUntilSuccess(lambda: self.login(pin, 0), int(repeat))

		if self.verbose:
			print('Trying to login...')

		response = self.sendJSON('https://'+self.camera_ip+'/auth/login', {"psw": pin})

		if response is None:
			print('Login failed.')
			return False

		self.token = json.load(self.u8reader(response))["token"]
		self.initCachedValues()

		if self.verbose:
			print('login succeeded with token ', self.token)

		return True

	def logout(self):
		if not self.requireToken():
			return False

		if self.verbose:
			print('Trying to logout...')

		response = self.sendJSON('https://'+self.camera_ip+'/auth/logout', {"token": self.token})

		if response is None:
			print('Logout failed.')
			return False

		return True

	def sendHeartbeat(self, loop=0):
		while True:

			if self.stop_heartbeat:
				break

			if self.token is None:
				if self.reconnect:
					print('Try to re-connect...')
					self.connect(self.pin)
				return False

			self.sendJSON('https://'+self.camera_ip+'/operation/HTBEAT', {"token": self.token})

			if loop==0:
				break
				
			time.sleep(loop) 

		self.stop_heartbeat = False
		return True

	def connect(self, pin="0000",token=None):
		self.pin = pin
		if token is not None:
			self.stop_heartbeat = True
			time.sleep(2)
			self.token = token
			self.initCachedValues()

		if token is not None or self.login(pin, self.login_tries):
			Heartbeat_thread = threading.Thread(target=self.sendHeartbeat, args=(1,), kwargs={})
			Heartbeat_thread.start()
			return True

		if(self.last_http_status is 421): print('Connection refused, too many clients')
		else: print('Connection refused.')

		return False
	
	def disconnect(self):
		self.stop_heartbeat = True
		self.logout()
		self.token = None
		if self.verbose:
			print('disconnected.')

	def setVideoResolution(self, resolution):
		if not self.requireToken():
			return False

		if resolution is "6480x1080":
			resolution = 0
		elif resolution is "3840x640":
			resolution = 1
		elif resolution is "2880x480":
			resolution = 2
		elif resolution is "1920x320":
			resolution = 3

		if self.sendJSON('https://'+self.camera_ip+'/settings/video/VDORES',{"token": self.token, "vdores": resolution}) is None:
			return False

		return True

	def setVideoWhitebalance(self, whitebalance):
		if not self.requireToken():
			return False

		if whitebalance is "auto":
			whitebalance = 0
		elif whitebalance is "tungsten":
			whitebalance = 1
		elif whitebalance is "fluroscent":
			whitebalance = 2
		elif whitebalance is "sunny":
			whitebalance = 3
		elif whitebalance is "cloudy":
			whitebalance = 4

		if self.sendJSON('https://'+self.camera_ip+'/settings/video/VDOWBL',{"token": self.token, "vdowbl": whitebalance}) is None:
			return False

		return True

	def setVideoEffect(self, effect):
		if not self.requireToken():
			return False

		if effect is "none":
			effect = 0
		elif effect is "mono":
			effect = 1
		elif effect is "sepia":
			effect = 2

		if self.sendJSON('https://'+self.camera_ip+'/settings/video/VDOCEF',{"token": self.token, "vdocef": effect}) is None:
			return False

		return True

	def setMode(self, mode):
		""" Set camera mode, 0: Photo, 1: Video, 2: Burst, 5: Timelapse, 6: Surveillance """
		if not self.requireToken():
			return False

		if mode is "photo":
			mode = 0
		elif mode is "video":
			mode = 1
		elif mode is "burst":
			mode = 2
		elif mode is "timelapse":
			mode = 5
		elif mode is "surveillance":
			mode = 6

		if self.sendJSON('https://'+self.camera_ip+'/operation/SWTMOD',{"token": self.token, "swtmod": mode}) is None:
			return False

		return True	

	def setAutoRotation(self, auto_rotation):
		if not self.requireToken():
			return False

		if self.sendJSON('https://'+self.camera_ip+'/settings/CAMFLP',{"camflp": auto_rotation, "token": self.token}) is None:
			return False

		return True	

	def setStream(self, stream):
		if not self.requireToken():
			return False

		if self.sendJSON('https://'+self.camera_ip+'/streaming/STREAM',{"stream": stream, "token": self.token}) is None:
			return False

		return True	

	def setStreamOffset(self, offset):
		if not self.requireToken():
			return False

		if self.sendJSON('https://'+self.camera_ip+'/streaming/OFFSET',{"offset": offset, "token": self.token}) is None:
			return False

		return True	

	def startVideo(self):
		if not self.setMode(1):
			return False
		if not self.setStream(True):
			return False

		return True

	def takePicture(self):
		""" Take a picture (make sure to call setMode(0) before) """
		if not self.requireToken():
			return False

		if self.sendJSON('https://'+self.camera_ip+'/operation/SHUBUT',{"token": self.token}) is None:
			return False

		return True
		

	def stopVideo(self):
		return self.setStream(False)

	def returnCached(self, field, key, require_permission=True):
		if self.token is None and require_permission is True:
			print('This operation is only available after sucessfully connecting to the camera.')
			return None

		return field.readElement(key)

	def getName(self):
		return self.returnCached(self.camera_info, ("caminf", "name"), False)

	def getBattery(self):
		return self.returnCached(self.camera_status, "batlev")

	def getHdmiConnected(self):
		return self.returnCached(self.camera_status, "hdmist")

	def getPowerConnected(self):
		return self.returnCached(self.camera_status, "powsrc")

	def getVideoResolution(self):
		return self.returnCached(self.video_settings, "vdores")

	def getVideoEffect(self):
		return self.returnCached(self.video_settings, "vdocef")

	def getVideoWhitebalance(self):
		return self.returnCached(self.video_settings, "vdowbl")

	def getVideoTimelapseInterval(self):
		return self.returnCached(self.video_settings, ("vdotlp", "interval"))

	def getPhotoBurstRate(self):
		return self.returnCached(self.photo_settings, ("picbur", "rate"))

	def getPhotoEffect(self):
		return self.returnCached(self.photo_settings, "piccef")

	def getPhotoExposure(self):
		return self.returnCached(self.photo_settings, "picexp")		

	def getPhotoResolution(self):
		return self.returnCached(self.photo_settings, "picres")

	def getPhotoTimelapseInterval(self):
		return self.returnCached(self.photo_settings, ("pictlp", "interval"))				

	def getPhotoWhitebalance(self):
		return self.returnCached(self.photo_settings, "picwbl")	

	def getPhotoPictwbInterval(self): # TODO: unknown setting, rename 
		return self.returnCached(self.photo_settings, ("pictwb", "interval"))

	def getPhotoPictwbRate(self): # TODO: unknown setting, rename
		return self.returnCached(self.photo_settings, ("pictwb", "rate"))	