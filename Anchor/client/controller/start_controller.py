# ===== IMPORTS =====
import signal
import sys
import time
import numpy as np
from ntb import client as ntbclient
from ntb import estimation as ntbestimator

import cflib.crtp
from cfclient.utils.logconfigreader import LogConfig
from cflib.crazyflie import Crazyflie

# ===== NETWORK SETTINGS =====
TCP_PORT = 23458
IP_ADDR_LIST = ('172.17.8.0','172.17.8.1','172.17.8.2', '172.17.8.3', '172.17.8.4', '172.17.8.5')
tcp_clients = []
anchor_positions = { \
		0:[7.111, 2.884, 2.419], \
		1:[7.094, 7.708, 2.419], \
		2:[1.606, 7.674, 2.419], \
		3:[1.614, 2.812, 2.419],
		4:[5.284, 5.303, 2.419],
		5:[3.442, 5.312, 2.419]  }

# ===== FILTER SETTINGS =====
ESTIMATE_PERIOD  = 0.100
NUM_PARTICLES    = 250
INNOVATION_NOISE = 0.15
ESTIMATE_VAR = 0.30
AREA_LIMITS = (10,10,2.60)
num_targets = 0

# ===== DETECTED MOBILE TARGETS =====
TARGET_TIMEOUT = 30
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
		tgt = ntbestimator.Target(srcAddr)
		estimator = ntbestimator.ParticleFilterPos(NUM_PARTICLES, INNOVATION_NOISE, AREA_LIMITS)
		tgt.setEstimator(estimator)

		# add to list and create a new particle filter
		mobile_targets[srcAddr] = tgt

	# get the position corresponding to this uid
	xyz_anchor = anchor_positions[uid]

	# add new range estimate to the appropriate target
	mobile_targets[srcAddr].addRangeMeasurement(uid, xyz_anchor, rangeEst, ESTIMATE_VAR)

	#print(uid, ',', rangeEst);

def cf_connected():
	""" This callback is called form the Crazyflie API when a Crazyflie
    has been connected and the TOCs have been downloaded."""
    print "Connected to %s" % link_uri

    # The definition of the logconfig can be made before connecting
    self._lg_stab = LogConfig(name="Stabilizer", period_in_ms=250)
    self._lg_stab.add_variable("stabilizer.yaw", "float")
    self._lg_stab.add_variable("stabilizer.pitch", "float")
    self._lg_stab.add_variable("stabilizer.roll", "float")
    self._lg_stab.add_variable("stabilizer.thrust", "uint16_t")

    # Adding the configuration cannot be done until a Crazyflie is
    # connected, since we need to check that the variables we
    # would like to log are in the TOC.
    self._cf.log.add_config(self._lg_stab)
    if self._lg_stab.valid:
        # This callback will receive the data
        self._lg_stab.data_received_cb.add_callback(self._stab_log_data)
        # This callback will be called on errors
        self._lg_stab.error_cb.add_callback(self._stab_log_error)
        # Start the logging
        self._lg_stab.start()
    else:
        print("Could not add logconfig since some variables are not in TOC")

def cf_disconnected():
	pass

def cf_failed():
	pass

def cf_lost():
	pass
	


# ===== MAIN ENTRY POINT =====
if __name__ == '__main__':
	# assign ctrl-c handler
	signal.signal(signal.SIGINT, sys_exit_handler)

	print('-----------------------------')
	print('     Networked Test Bed      ')
	print('         Controller          ')
	print('-----------------------------')

	# connect to cflie
    cflib.crtp.init_drivers(enable_debug_driver=False)
    # Scan for Crazyflies and use the first one found
    print("Scanning interfaces for Crazyflies...")
    available = cflib.crtp.scan_interfaces()
    print("Crazyflies found:")
    for i in available:
        print(i[0])

    if len(available) == 0:
        print("No Crazyflies found, exiting")
        sys.exit(0)

    cflie = Crazyflie()
    cflie_uri = 'radio://0/80/250K'
    cflie.connected.add_callback(cf_connected)
    cflie.disconnected.add_callback(cf_disconnected)
    cflie.connection_failed.add_callback(cf_failed)
    fclie.connection_lost.add_callback(cf_lost)
    print("Connecting to cflie: ", cflie_uri)
    # Try to connect to the Crazyflie
    cflie.open_link(cflie_uri)

    # Variable used to keep main loop occupied until disconnect
    cflie_connected = True

	# initialize all TCP clients
	print('Initializing TCP clients:')
	for addr in IP_ADDR_LIST:
		clnt = ntbclient.TcpClient( (addr,TCP_PORT) )
		if clnt.getStatus() == 'Connected':
			clnt.setRangeCallback(handleNewRangeEst)
			tcp_clients.append(clnt)
			clnt.streamRanges(True)
			clnt.start()

		print('   - Client at (', addr, ', ', TCP_PORT, '): ', clnt.getStatus())

	# loop forever, periodically updating position estimates
	while(1):
		time.sleep(ESTIMATE_PERIOD)
		for addr in mobile_targets:
			mobile_targets[addr].updateEstimate()
			# print xyz estimates and then all ranges
			xyz_est = mobile_targets[addr].getEstimate()
			print(xyz_est[0], ',', xyz_est[1], ',', xyz_est[2])


	



