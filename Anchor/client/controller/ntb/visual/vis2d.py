# ===== IMPORTS =====
import numpy as np
import time
import matplotlib.pyplot as plt
import matplotlib.animation as animation


# ===== CONSTANTS =====
TARGET_PAST_HISTORY = 100

# ===== VISUALIZER CLASS =====
class Visualizer:
	def __init__(self, fsize=(20,10), limits=(10,10,10)):
		self.fig = plt.figure()

		# initialize figure
		plt.ion()

		self.fig = plt.figure()
		self.ax = self.fig.add_subplot(111)
		self.ax.set_xlabel('X Position (m)')
		self.ax.set_ylabel('Y Position (m)')
		self.ax.set_xlim(0, limits[0])
		self.ax.set_ylim(0, limits[1])

		# anchor plots
		self.anchors_plt = None
		self.anchor_ids = []
		self.anchor_xyz = []

		# target plots
		self.target_x = [0 for x in range(TARGET_PAST_HISTORY)]
		self.target_y = [0 for x in range(TARGET_PAST_HISTORY)]
		self.target_plt, = self.ax.plot(self.target_x, self.target_y, 'r-')

		self.targets = {}

		# node objects
		self.nodes = {}
		self.targets = {}

		# set animation callback
		self.ani = animation.FuncAnimation(self.fig, self.animate, interval=100, fargs=(self, ))

		# show plot
		plt.show()

	def __del__(self):
		pass

	def setAxes(self, axes):
		pass

	def setAzimuth(self, angle):
		pass

	def setZenith(self, angle):
		pass

	def setZoom(self, zoom):
		pass

	def addNode(self, uid, xyz):
		pass

	def updateTarget(self, xyz_tgt):
		# update buffers
		self.target_x.append(xyz_tgt[0])
		self.target_y.append(xyz_tgt[1])
		if len(self.target_x) > TARGET_PAST_HISTORY:
			self.target_x.pop(0)
			self.target_y.pop(0)

	def animate(i, self):
		# update plot data
		self.target_plt.set_xdata(self.target_x)
		self.target_plt.set_ydata(self.target_y)
		plt.draw()


	def removeTarget(self, nodeId):
		pass

	def plotParticles(self, xyz_p, est):
		pass




# ===== STANDALONE TESTS =====
if __name__ == '__main__':
	vis = Visualizer( (10,10), (10,10,2.504) )
	vis.updateTarget( (1, 1, 1) )
	vis.updateTarget( (2, 1, 1) )
	vis.updateTarget( (3, 3, 1) )
	vis.updateTarget( (4, 5, 1) )
	vis.updateTarget( (6, 3, 1) )
	vis.updateTarget( (7, 2, 1) )



	while(1):
		time.sleep(0.01)

