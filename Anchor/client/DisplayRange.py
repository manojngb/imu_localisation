# -- Imports --
# -- imports --
import os
import re
import serial
import time
import numpy as np
import collections

# distance conversion
dist_zero = 7390000

# serial port
baud_rate = 115200
dev_path = '/dev/'
dev_name = 'ttyACM0'
dev = ''

past_dist = 0
alpha = 0.05

dev = serial.Serial(dev_path+dev_name, baudrate=baud_rate, timeout=0.050)

def flush():
	dev.flushInput()
	dev.flush()

while True:
	# add some delays
	time.sleep(0.050)

	# tell the device to send us some data
	dev.write(' ')

	# read new line
	line = dev.readline()
	try:
		decodeline = line.decode("utf-8")
		dist_cm = int(line) - dist_zero
		dist_m = dist_cm/100000.0
		dist_ave = dist_m*alpha + past_dist*(1-alpha)
		past_dist = dist_ave
		print "Distance: %f, AVE10: %f" % (dist_m, dist_ave)
	except:
		pass
		#print('skipping bad packet')
		#flush()






		


	



