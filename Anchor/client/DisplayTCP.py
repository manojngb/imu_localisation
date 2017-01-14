# -- TCP --
import socket
import sys

# -- imports --
import signal
import os
import re
import serial
import time
import numpy as np
import collections
import struct
import matplotlib.pyplot as plt

N = 200

LOS_THRESH = -8.0
LOS_SAMPTHRESH = 5
LOS = False
LOS_good = 0

plt.subplot(2,1,1)
plt.ylim([0, 12])
plt.xlim([0,N])
plt.ylabel('Distance (m)')
plt.xlabel('Time (s)')
graph = plt.plot(0,0)[0]

plt.subplot(2,1,2)
plt.ylim([-25, 5])
plt.xlim([0,N])
plt.ylabel('Path Loss (dB)')
plt.xlabel('Time (s)')
plgraph = plt.plot(0,0)[0]

plt.ion()
plt.show()
history = []
plhistory = []

past_dist = 0
alpha = 0.30

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = (sys.argv[1], 23458)

# ===== HANDLE CTRL-C =====
def sys_exit_handler(signal, frame):
	print('\n----- KEYBOARD INTERRUPT ----- ')
	print('Cleaning up TCP clients:')
	sock.close()
	# exit the program
	sys.exit(0)

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


# assign ctrl-c handler
signal.signal(signal.SIGINT, sys_exit_handler)

# say hey to the server
print 'connecting to %s port %s' % server_address
sock.connect(server_address)
try:
	# start streaming
	values = (ord('R'), ord('s'), ord('0'))
	packer = struct.Struct('! B B B')
	packed_data = packer.pack(*values)

	print 'starting streaming...'
	sock.sendall(packed_data)

	for line in readlines(sock):
		try:
			tokens = line.split(',')
			dist_cm = int(tokens[2])
			dist_m = dist_cm/100.0
			pathloss = int(tokens[3])/100.0
			#dist_m = dist_cm;

			if pathloss < LOS_THRESH:
				LOS = False
				LOS_good = 0
			else:
				LOS_good += 1
				if LOS_good > LOS_SAMPTHRESH:
					LOS = True

			if len(history) < N:
				history.extend([dist_m])
				plhistory.extend([pathloss])
			else:
				history.pop(0)
				history.extend([dist_m])
				plhistory.pop(0)
				plhistory.extend([pathloss])

			print "Distance: %.2f\tAverage: %.2f\t95%% Conf: +/-%.2f cm Loss: %.2f" % (dist_m, np.mean(history), 2*100*np.std(history), pathloss)
			graph.set_ydata(history)
			times = range(0,len(history))
			graph.set_xdata(times)

			plgraph.set_ydata(plhistory)
			plgraph.set_xdata(times)

			plt.draw()


		except Exception as e:
			print e
			print "Non-numeric response: %s" % line

finally:
	print 'closing socket'
	sock.close()




		


	



