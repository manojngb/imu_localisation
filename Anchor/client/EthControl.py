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


# get host name from command line
ntb_host = sys.argv[1]
server_address = (''.join(['ntb-', ntb_host]), 23458)

# say hey to the server
print 'connecting to %s port %s' % server_address
sock.connect(server_address)

# reset switch
values = (ord('E'), ord('r'), 0)
packer = struct.Struct('! B B B')
packed_data = packer.pack(*values)
print 'sending "%s"' % (values,)
sock.sendall(packed_data)
time.sleep(10)

# turn led on
values = (ord('E'), ord('l'), 1)
packer = struct.Struct('! B B B')
packed_data = packer.pack(*values)
print 'sending "%s"' % (values,)
sock.sendall(packed_data)
time.sleep(1)

# CFG 1 register
# working: values = (ord('E'), ord('1'), int('01111001', 2))
values = (ord('E'), ord('1'), int('01111001', 2))
packer = struct.Struct('! B B B')
packed_data = packer.pack(*values)
print 'sending "%s"' % (values,)
sock.sendall(packed_data)
time.sleep(1)

# CFG 2 register low
values = (ord('E'), ord('2'), int('000000100', 2))
packer = struct.Struct('! B B B')
packed_data = packer.pack(*values)
print 'sending "%s"' % (values,)
sock.sendall(packed_data)
time.sleep(1)

# CFG 2 register high
values = (ord('E'), ord('3'), int('00000000', 2))
packer = struct.Struct('! B B B')
packed_data = packer.pack(*values)
print 'sending "%s"' % (values,)
sock.sendall(packed_data)
time.sleep(1)








		


	



