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

# -- IMPORTS --
import socket
import threading
import time
import struct
import traceback

# -- ANCHOR TCP COMMAND DEFINITIONS --
# Subsystem definitions
NTB_SYS_CTRL  =ord('C')
NTB_SYS_PCTRL =ord('P')
NTB_SYS_PMEAS =ord('M')
NTB_SYS_LED   =ord('L')
NTB_SYS_UAR   =ord('U')
NTB_SYS_UID   =ord('I')
NTB_SYS_GPIO  =ord('G')
NTB_SYS_RANGE =ord('R')
NTB_NULL      = 0

# System Control Commands
NTB_CTRL_RST =ord('r')

# Power Control Commands
# PCTRL commands
NTB_PCTRL_POWOFF =ord('0')
NTB_PCTRL_POWON  =ord('1')
NTB_PCTRL_POWRST =ord('2')
# PCTRL options
NTB_POW_3V3 =ord('3')
NTB_POW_5V0 =ord('5')

# Power Measurement Commands
# PMEAS commands
NTB_PMEAS_STREAM_1HZ   =ord('1')
NTB_PMEAS_STREAM_10HZ  =ord('2')
NTB_PMEAS_STREAM_100HZ =ord('3')
# PMEAS options - same as PCTRL

# LED Commands
NTB_LED_OFF    =ord('0')
NTB_LED_ON     =ord('1')
NTB_LED_TOGGLE =ord('t')

# LED options
NTB_LED_ARM_GREEN =ord('g')
NTB_LED_ARM_RED   =ord('r')
NTB_LED_ARM_BLUE  =ord('b')

# UID Commands
NTB_UID_READ =ord('r')

# GPIO Commands
NTB_GPIO_OFF    =ord('0')
NTB_GPIO_ON     =ord('1')
NTB_GPIO_TOGGLE =ord('t')

# GPIO Options
NTB_GPIO_0   =ord('0')
NTB_GPIO_1   =ord('1')
NTB_GPIO_2   =ord('2')
NTB_GPIO_3   =ord('3')
NTB_GPIO_RST =ord('r')

# UART Commands
NTB_UART_STREAM =ord('s')
NTB_UART_BAUD   =ord('b')
# UART Options
NTB_UART_DBGALL =ord('a')
NTB_UART_DBG_1  =ord('1')
NTB_UART_DBG_2  =ord('2')

# RANGE Commands
NTB_RANGE_STARTSTREAM =ord('s')
NTB_RANGE_STOPSTREAM  =ord('e')

# -- TCP SOCKET DEFINITIONS --
TCP_RECV_BUFFER_SIZE = 128
TCP_RECV_TIMEOUT_MS = 200

# =========== Socket Subroutines =============
def readline(sock, recv_buffer=64, delim='\n'):
	buffer = ''
	data = True
	while data:
		data = sock.recv(recv_buffer)
		buffer += data.decode("utf-8")

		if buffer.find(delim) != -1:
			line, buffer = buffer.split('\n', 1)
			return line

def readlines(sock, recv_buffer=64, delim='\n'):
	buffer = ''
	data = True
	while data:
		data = sock.recv(recv_buffer)
		buffer += data.decode("utf-8")

		while buffer.find(delim) != -1:
			line, buffer = buffer.split('\n', 1)
			yield line
	return

def printRange(uid, srcAddr, rangeEst):
	print('uid=',uid,' src=',srcAddr,' range=',rangeEst)


# =========== NTB TCP Client Class =============
class TcpClient(threading.Thread):

	# Constructor
	def __init__(self, addr):
		# call thread constructor
		threading.Thread.__init__(self, name='client thread: ' + addr[0])

		# Dynamic variables
		self.addr = addr # (ip, port)
		self.uid = -1
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.sock.settimeout(TCP_RECV_TIMEOUT_MS/1000.0)
		self.status = 'Idle'
		self.terminated = threading.Event()

		# streaming callback functions
		self.range_callback = None
		self.power_callback = None

		# attempt to connect
		try:
			self.sock.connect( self.addr )
			self.status = 'Connected'
		except Exception as e:
			self.status = 'Error'

	def __del__(self):
		pass

	# Subroutines
	def stop(self):
		self.terminated.set()

	def stopped(self):
		return self.terminated.isSet()

	def setRangeCallback(self, cb):
		self.range_callback = cb

	def setPowerCallback(self, cb):
		self.power_callback = cb

	def getStatus(self):
		return self.status

	def getAddress(self):
		return self.addr

	def getUid(self):
		if self.status != 'Connected':
			return

		cmd = (NTB_SYS_UID, NTB_UID_READ, NTB_NULL)
		packer = struct.Struct('! B B B')
		packed_data = packer.pack(*cmd)
		self.sock.sendall(packed_data)
		uid = readline(self.sock)
		return int(uid)

	def streamRanges(self, stream):
		if self.status != 'Connected':
			return

		if stream:
			cmd = (NTB_SYS_RANGE, NTB_RANGE_STARTSTREAM, NTB_NULL)
		else:
			cmd = (NTB_SYS_RANGE, NTB_RANGE_STOPSTREAM, NTB_NULL)

		packer = struct.Struct('! B B B')
		packed_data = packer.pack(*cmd)
		self.sock.sendall(packed_data)

	def parseData(self, data):
		# split into CSV tokens
		tokens = data.split(',')
		if len(tokens) < 3:
			return

		uid = int(tokens[0])

		# parse range data
		if len(tokens) == 4:
			srcAddr = int(tokens[1])
			rangeEst = int(tokens[2])/100.0
			pathLoss = int(tokens[3])/100.0

			# drop range estimate if path loss is too great
			if pathLoss > -10: # (dB)
				if self.range_callback != None:
					self.range_callback(uid, srcAddr, rangeEst-0.550)
				return

	def close(self):
		if self.status != 'Connected':
			return
		self.sock.close();

	def run(self):
		if self.status != 'Connected':
			return

		while(1):
			try:
				if self.terminated.isSet():
					break
				for line in readlines(self.sock):
					if self.terminated.isSet():
						break
					self.parseData(line)

			except Exception as e:
				if self.terminated.isSet():
					break
				pass

		# clean up the socket
		self.close()


	
# ========== Client Standalone Test ===========
if __name__ == '__main__':
	clnt = TcpClient( ('localhost', 23458) )
	print("attempting to stream ranges ... ")
	clnt.setRangeCallback(printRange)
	clnt.streamRanges()
	clnt.start()

