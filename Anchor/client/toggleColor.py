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
PORT = 23458
IP_ADDR_LIST = ('172.17.8.0','172.17.8.1','172.17.8.2', '172.17.8.3', '172.17.8.4', '172.17.8.5')

if __name__ == '__main__':

	for addr in IP_ADDR_LIST:
		sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		sock.settimeout(0.050)
		server_address = (addr, PORT)
		print 'connecting to %s port %s' % server_address
		sock.connect(server_address)

		try:
			values = (ord('L'), ord('0'), ord('G'))
			packer = struct.Struct('! B B B')
			packed_data = packer.pack(*values)
			sock.sendall(packed_data)

			time.sleep(0.1)

			values = (ord('L'), ord('0'), ord('R'))
			packer = struct.Struct('! B B B')
			packed_data = packer.pack(*values)
			sock.sendall(packed_data)

			time.sleep(0.1)

			values = (ord('L'), ord('0'), ord('B'))
			packer = struct.Struct('! B B B')
			packed_data = packer.pack(*values)
			sock.sendall(packed_data)

			time.sleep(0.1)

			values = (ord('L'), ord('0'), ord('O'))
			packer = struct.Struct('! B B B')
			packed_data = packer.pack(*values)
			sock.sendall(packed_data)

		except:
			sock.close()
			continue

		sock.close()




		


	



