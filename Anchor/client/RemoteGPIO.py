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

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.settimeout(0.050)
server_address = ('172.17.5.15', 23458)

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

values = (ord('G'), ord('t'), ord('2'))
packer = struct.Struct('! B B B')
packed_data = packer.pack(*values)

print 'sending "%s"' % (values,)
sock.sendall(packed_data)
try:
	resp = sock.recv(32)
	print 'response: "%s"' % resp
except:
	pass




		


	



