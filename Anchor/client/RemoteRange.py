# -- TCP --
import socket
import sys

# -- imports --
import os
import re
import serial
import time
import numpy as np
import collections
import struct
import matplotlib.pyplot as plt

N = 50

plt.ylim([0, 12])
plt.xlim([0,N])
plt.ylabel('Distance (m)')
plt.xlabel('Time (s)')
graph = plt.plot(0,0)[0]
#plt.ion()
#plt.show()
history = []

# serial port
baud_rate = 115200
dev_path = '/dev/'
dev_name = 'ttyACM0'
dev = ''

past_dist = 0
alpha = 0.30

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.settimeout(1.000)
server_address = ('172.17.5.15', 23458)
MAX_LINES = 50

def readlines(sock, recv_buffer=64, delim='\n'):
	buffer = ''
	data = True
	while data:
		data = sock.recv(recv_buffer)
		buffer += data

		while buffer.find(delim) != -1:
			line, buffer = buffer.split('\n', 1)
			yield line
	return




# say hey to the server
print 'connecting to %s port %s' % server_address
sock.connect(server_address)
try:
	# init streaming
	values = ('R', 's', 0)
	packer = struct.Struct('! B B B')
	packed_data = packer.pack(*values)
	print 'sending "%s"' % (values,)
	sock.sendall(packed_data)

	lines = 0

	for line in readlines(sock):
		try:
			print(line)
			lines = lines + 1
			if lines >= MAX_LINES:
				break


		except Exception as e:
			print e

finally:
	values = ('R', 'e', 0)
	packer = struct.Struct('! B B B')
	packed_data = packer.pack(*values)
	print 'sending "%s"' % (values,)
	sock.sendall(packed_data)

	print 'closing socket'
	sock.close()




		


	



