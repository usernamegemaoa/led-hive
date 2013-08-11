#!/usr/bin/env python
import os
from select import poll, POLLIN, POLLPRI, POLLERR
from time import time, sleep

class GPIO:
	def __init__(self, pin, direction='out'):
		self.pin = str(pin)
		self.export()
		self.set_direction(direction)
	
	def __del__(self):
		self.unexport()

	def export(self):
		try:
			fp = open("/sys/class/gpio/export","w")
			fp.write(self.pin)
			fp.close()
		except IOError:
			print "GPIO %s already Exists, so skipping export gpio" % (self.pin, )

	def unexport(self):
		try:
			fp = open("/sys/class/gpio/unexport","w")
			fp.write(self.pin)
			fp.close()
		except IOError:
			print "GPIO %s not found, so skipping unexport gpio" % (self.pin, )

	def set_direction(self, direction='out'):
		fp = open("/sys/class/gpio/gpio%s/direction" % (self.pin,), 'w')
		fp.write(direction)
		fp.close()		

	def value(self):
		fp = open("/sys/class/gpio/gpio%s/value" % (self.pin,), 'r')
		v = fp.read()
		fp.close()
		return v

	def set(self, v):
		fp = open("/sys/class/gpio/gpio%s/value" % (self.pin,), 'w')
		fp.write(v)
		fp.close()
		return v
	
	def strobe(self, duration = 0.001):
		self.set("1")
		sleep(duration)
		self.set("0")

	def poll(self, cb, timeout = None):
		fp = open("/sys/devices/virtual/gpio/gpio%s/edge" % (self.pin,), 'w')
		fp.write("both")
		fp.close()

		f = file('/sys/devices/virtual/gpio/gpio%s/value' % (self.pin,))
		p = poll()
		p.register(f.fileno(), POLLPRI)
		lt = time()
		while True:
			events = p.poll(timeout)
			v = None
			print events
			if events:
				v = os.read(f.fileno(), 1)
				f.seek(0)
				llt = time()
			if cb(v, time() - lt): break
			lt = llt
