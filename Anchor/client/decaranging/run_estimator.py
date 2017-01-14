"""
UCLA NESL Networked Test Bed (NTB) DW1000 Ranging estimation
Author: Paul Martin
Email: pdmartin@ucla.edu

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

# ===== IMPORTS =====
import signal
import sys
import time
import numpy as np
from anchorlib import client
from anchorlib import estimation


# ===== NETWORK SETTINGS =====
TCP_PORT = 23458
IP_ADDR_LIST = ('172.17.8.0','172.17.8.1','172.17.8.2', '172.17.8.3', '172.17.8.4', '172.17.8.5')
tcp_clients = []
anchor_positions = {
		0:[7.111, 2.884, 2.419],
		1:[7.094, 7.708, 2.419],
		2:[1.606, 7.674, 2.419],
		3:[1.614, 2.812, 2.419],
		4:[5.284, 5.303, 2.419],
		5:[3.442, 5.312, 2.419]  }

# ===== FILTER SETTINGS =====
ESTIMATE_PERIOD  = 0.100
NUM_PARTICLES    = 200
INNOVATION_NOISE = 0.10
ESTIMATE_VAR = 0.30
AREA_LIMITS = (10,10,2.40)
num_targets = 0

# ===== DETECTED MOBILE TARGETS =====
mobile_targets = {}

# ===== HANDLE CTRL-C =====
def sys_exit_handler(signal, frame):
	print('\n----- KEYBOARD INTERRUPT ----- ')
	print('Cleaning up TCP clients:')
	for clnt in tcp_clients:
		addr = clnt.getAddress()
		print('   - Closing client at (', addr[0], ', ', addr[1], ')')
		clnt.stop()
		while not clnt.stopped():
			pass
		clnt.join()
		del clnt
	print('Cleaning up targets & filters:')
	print('   - Removing ', len(mobile_targets), ' target(s)')
	mobile_targets.clear()
	# exit the program
	sys.exit(0)

# ===== HANDLE RANGE ESTIMATES =====
def handleNewRangeEst(uid, srcAddr, rangeEst):
	#check for new mobile_targets
	if srcAddr not in mobile_targets:
		print('Adding target with id (', srcAddr,')')
		# new mobile target detected: create target object with an estimator
		tgt = estimation.Target(srcAddr)
		estimator = estimation.ParticleFilter(NUM_PARTICLES, INNOVATION_NOISE, AREA_LIMITS)
		tgt.setEstimator(estimator)

		# add to list and create a new particle filter
		mobile_targets[srcAddr] = tgt

	# get the position corresponding to this uid
	xyz_anchor = anchor_positions[uid]

	# add new range estimate to the appropriate target
	mobile_targets[srcAddr].addRangeMeasurement(uid, xyz_anchor, rangeEst, ESTIMATE_VAR)
	

# ===== MAIN ENTRY POINT =====
if __name__ == '__main__':

	# assign ctrl-c handler
	signal.signal(signal.SIGINT, sys_exit_handler)

	print('-----------------------------')
	print('    DW1000 Anchor Particle   ')
	print('          Filter             ')
	print('-----------------------------')

	# initialize all TCP clients
	print('Initializing TCP clients:')
	for addr in IP_ADDR_LIST:
		clnt = client.TcpClient( (addr,TCP_PORT) )
		if clnt.getStatus() == 'Connected':
			clnt.setRangeCallback(handleNewRangeEst)
			tcp_clients.append(clnt)
			clnt.streamRanges(True)
			clnt.start()

		print('   - Client at (', addr, ', ', TCP_PORT, '): ', clnt.getStatus())

	# loop forever, periodically updating position estimates (should really be using a timer here)
	while(1):
		time.sleep(ESTIMATE_PERIOD)

		for addr in mobile_targets:
			mobile_targets[addr].updateEstimate()
			# print xyz estimates and then all ranges
			xyz_est = mobile_targets[addr].getEstimate()
			print('[', addr, ']: ', xyz_est[0], ',', xyz_est[1], ',', xyz_est[2])


	



