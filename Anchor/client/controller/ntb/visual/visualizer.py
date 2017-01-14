# ===== IMPORTS =====
import numpy as np
import time
from mpl_toolkits.mplot3d import Axes3D
import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
import matplotlib.animation as animation


# ===== CONSTANTS =====
TARGET_PAT_HISTORY = 100

# ===== VISUALIZER CLASS =====
class Visualizer:
	def __init__(self, fsize=(20,10), limits=(10,10,10)):
		self.fig = plt.figure()

		# initialize figure
		plt.ion()
		self.fig = plt.figure()
		self.fig.set_size_inches(fsize[0], fsize[1], forward=True)
		self.ax = self.fig.add_subplot(111, projection='3d')
		self.ax.set_xlabel('X Position (m)')
		self.ax.set_ylabel('Y Position (m)')
		self.ax.set_zlabel('Z Position (m)')
		self.ax.set_xlim3d(0, limits[0])
		self.ax.set_ylim3d(0, limits[1])
		self.ax.set_zlim3d(0, limits[2])

		# anchor plots
		self.anchors_plt = None
		self.anchor_ids = []
		self.anchor_xyz = []

		# target plots
		self.target_plt = self.ax.scatter([], [], [], c='b', marker='o', s=100)
		self.targets = {}

		# node objects
		self.nodes = {}
		self.targets = {}

	def __del__(self):
		pass

	def show(self):
		plt.show()

	def setAxes(self, axes):
		pass

	def setAzimuth(self, angle):
		pass

	def setZenith(self, angle):
		pass

	def setZoom(self, zoom):
		pass

	def addNode(self, uid, xyz):
		if uid not in self.anchor_ids:
			self.anchor_ids.append(uid)
			self.anchor_xyz.append(xyz)

			if self.anchors_plt is not None:
				self.ax.collections.remove(self.anchors_plt)

			xdata = [p[0] for p in self.anchor_xyz]
			ydata = [p[1] for p in self.anchor_xyz]
			ydata = [p[2] for p in self.anchor_xyz]
			self.ax.scatter(xdata, ydata, ydata, c='r', marker='o', s=100)
			plt.draw()

	def updateTarget(self, xyz_tgt, xyz_particles, color):
		pass

	def removeTarget(self, nodeId):
		pass

	def plotParticles(self, xyz_p, est):
		ax = self.fig.add_subplot(111, projection='3d')
		xs = [x[0] for x in xyz_p]
		ys = [x[1] for x in xyz_p]
		zs = [x[2] for x in xyz_p]
		ax.scatter(xs, ys, zs, c='b', marker='o', s=20)
		ax.scatter(est[0], est[1], est[2], c='r', marker='s', s=100)

		ax.set_xlabel('X Label')
		ax.set_ylabel('Y Label')
		ax.set_zlabel('Z Label')

		plt.show()





# ===== STANDALONE TESTS =====
if __name__ == '__main__':
	vis = Visualizer( (10,10), (10,10,5) )
	# p = [[1,1,2], [4,6,3], [2,8,5]]
	# e = [5,5,2]
	# vis.plotParticles(p,e)
	# time.sleep(1)
	# p = [[3,1.5,2.5], [3,5.7,3.1], [2.2,7.7,4.8]]
	# e = [4,3,3.3]
	# vis.plotParticles(p,e)
	vis.addNode(1, [3,5,4])
	vis.addNode(2, [6,1,4.2])

	while(1):
		plt.pause(0.01)

