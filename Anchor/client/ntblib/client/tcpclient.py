# -- IMPORTS --
import socket
import threading
import time
import struct
import traceback

# -- NTB COMMAND DEFINITIONS --
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

	# Static Variables

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
		self.beacon_callback = None

		# beacon sequencing
		#self.bseq_last = 0
		#self.bseq_ovr = 0

		# file io
		self.fid = None

		# attempt to connect
		self.sock.connect( self.addr )
		self.status = 'Connected'

	def __del__(self):
		pass

	# Subroutines
	def stop(self):
		self.terminated.set()

	def stopped(self):
		return self.terminated.isSet()

	def setRangeCallback(self, cb):
		self.range_callback = cb

	def setBeaconCallback(self, cb):
		self.beacon_callback = cb

	def setPowerCallback(self, cb):
		self.power_callback = cb

	def writeToFile(self, fp):
		self.fid = open(fp, 'w')

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
		msgType = int(tokens[1])
		srcAddr = int(tokens[2])

		# handle ranges
		if msgType == 0:
			seqNum = int(tokens[3])
			ts1 = int(tokens[4])
			ts2 = int(tokens[5])
			ts3 = int(tokens[6])
			ts4 = int(tokens[7])
			ts5 = int(tokens[8])
			ts6 = int(tokens[9])
			fppwr = -int(tokens[10])/100.0
			cirp = int(tokens[11])
			fploss = -int(tokens[12])/100.0
			
			if self.range_callback != None:
				self.range_callback(fp, uid, seqNum, srcAddr, (ts1, ts2, ts3, ts4, ts5, ts6), fppwr, cirp, fploss)

			if self.fid != None:
				tposix = time.time()
				self.fid.write(str(tposix) + ',' + str(uid) + ',' + str(srcAddr) + ',' + str(seqNum) + ',' + str(ts1) + ',' + str(ts2) + ',' + str(ts3) + ',' + str(ts4) + ',' + str(ts5) + ',' + str(ts6) + ',' + str(fppwr) + ',' + str(cirp) + ',' + str(fploss) + '\n')

		# handle beacons
		if msgType == 1:
			seqNum = int(tokens[3])
			#if seqNum < self.bseq_last:
			#	self.bseq_ovr += 1
			#self.bseq_last = seqNum

			txtime = int(tokens[8]) + (int(tokens[7])<<8) + (int(tokens[6])<<16) + (int(tokens[5])<<24) + (int(tokens[4])<<32)
			rxtime = int(tokens[13]) + (int(tokens[12])<<8) + (int(tokens[11])<<16) + (int(tokens[10])<<24) + (int(tokens[9])<<32)
			fppwr = int(tokens[14])/100.0
			cirp = int(tokens[15])
			fploss = int(tokens[16])/100.0

			if self.beacon_callback != None:
				self.beacon_callback(fp, uid, srcAddr, seqNum + 256*self.bseq_ovr, txtime, rxtime, fppwr, cirp, fploss)

			if self.fid != None:
				self.fid.write(str(uid) + ',' + str(srcAddr) + ',' + str(seqNum) + ',' + str(txtime) + ',' + str(rxtime) + ',' + str(fppwr) + ',' + str(cirp) + ',' + str(fploss) + '\n')


	def close(self):
		if self.fid != None:
			self.fid.close()
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
	#uid = clnt.getUid()
	#print("client uid = ", uid)
	print("attempting to stream ranges ... ")
	#clnt.streamRanges()
	clnt.setRangeCallback(printRange)
	clnt.start()

