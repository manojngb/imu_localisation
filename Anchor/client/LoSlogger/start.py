#!/usr/bin/python3

# ===== IMPORTS =====
import signal
import sys
import os
import time
import datetime
import numpy as np
from ntb import client as ntbclient

import cflib.crtp
from cfclient.utils.logconfigreader import LogConfig
from cflib.crazyflie import Crazyflie

# ===== NETWORK SETTINGS =====
TCP_PORT = 23458
IP_ADDR_LIST = ('ntb-alpha','ntb-bravo','ntb-charlie','ntb-delta','ntb-echo','ntb-foxtrot')
tcp_clients = []

# ===== FILE OUTPUT =====
folder = ''.join(['/home/paul/Dropbox/Projects/MobileRadar/Data/', str(datetime.datetime.now())])
print('logging to: ', folder)
if not os.path.exists(folder):
    os.makedirs(folder)
fid_ranges = open(''.join([folder, '/rangeEstimates.log']), 'w')
fid_euler = open(''.join([folder, '/eulerAngles.log']), 'w')
fid_imu = open(''.join([folder, '/imuMeasurements.log']), 'w')

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
	# disconnect cflie
	cflie.close_link()
	# close files
	fid_ranges.close()
	fid_imu.close()
	fid_euler.close()
	# exit the program
	sys.exit(0)

# ===== HANDLE RANGE ESTIMATES =====
def handleNewRangeEst(uid, srcAddr, rangeEst, pathLoss):
	# write to the log
	fid_ranges.write('%.4f, %d, %d, %.3f, %d\n' % (time.time(), uid, srcAddr, rangeEst, pathLoss))

def handleCflieStabilizer(index, data, config_obj):
	fid_euler.write('%.4f, %.4f, %.4f, %.4f\n' % (time.time(), data['stabilizer.yaw'], data['stabilizer.pitch'], data['stabilizer.roll']))

def handleCflieImu(index, data, config_obj):
	fid_imu.write('%.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f\n' % (time.time(), data['acc.x'], data['acc.y'], data['acc.z'], data['gyro.x'], data['gyro.y'], data['gyro.z']))

def cf_connected(link_uri):
	print('Connected to', link_uri)

	# add stabilizer log block to cflie
	cflie.log.add_config(lg_stab)
	if lg_stab.valid:
		lg_stab.data_received_cb.add_callback(handleCflieStabilizer)
		lg_stab.start()

	# add imu log block to cflie
	cflie.log.add_config(lg_imu)
	if lg_imu.valid:
		lg_imu.data_received_cb.add_callback(handleCflieImu)
		lg_imu.start()


def cf_disconnected(link_uri):
	print('Disconnected from', link_uri)

def cf_failed():
	pass

def cf_lost(link_uri, reason):
	print('Lost crazyflie: ', link_uri, '--', reason)


# ===== MAIN ENTRY POINT =====
if __name__ == '__main__':
	# assign ctrl-c handler
	signal.signal(signal.SIGINT, sys_exit_handler)

	print('-----------------------------')
	print('     LoS Logging Utility     ')
	print('-----------------------------')

	# connect to cflie
	cflib.crtp.init_drivers(enable_debug_driver=False)
	# Scan for Crazyflies and use the first one found
	print("Scanning interfaces for Crazyflies...")
	available = cflib.crtp.scan_interfaces()
	print("BT Devices found:")
	for i in available:
		print(i[0])

	if len(available) == 0:
		print("No Crazyflies found, exiting")
		sys.exit(0)

	# logging on crazyflie
	lg_stab = LogConfig("Stabilizer", period_in_ms=20)
	lg_stab.add_variable("stabilizer.yaw", "float")
	lg_stab.add_variable("stabilizer.pitch", "float")
	lg_stab.add_variable("stabilizer.roll", "float")
	lg_stab.add_variable("stabilizer.thrust", "uint16_t")

	lg_imu = LogConfig("Imu", period_in_ms=20)
	lg_imu.add_variable("acc.x", "float")
	lg_imu.add_variable("acc.y", "float")
	lg_imu.add_variable("acc.z", "float")
	lg_imu.add_variable("gyro.x", "float")
	lg_imu.add_variable("gyro.y", "float")
	lg_imu.add_variable("gyro.z", "float")

	cflie = Crazyflie()
	cflie_uri = 'radio://0/80/250K'
	cflie.connected.add_callback(cf_connected)
	cflie.disconnected.add_callback(cf_disconnected)
	cflie.connection_failed.add_callback(cf_failed)
	cflie.connection_lost.add_callback(cf_lost)
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

	# loop forever
	while(1):
		time.sleep(0.01)


	



