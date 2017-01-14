# ----- IMPORTS -----
import numpy as np
import scipy
import scipy.stats
import random

WINDOW_PERIOD_SEC = 3.0

# ======== PARTICLE FILTER CLASS =========
class ParticleFilterYaw:

	def __init__(self, num_particles=100, innovate=1.0):
		self.Np = num_particles
		self.innovation = innovate
		self.meas_noise = 10.0*np.pi/180.0
		self.L_particles = np.zeros(num_particles)
		self.posHistory = []
		self.rpyHistory = []

		# initialize particle positions & target estimate
		self.yaw_particles = [np.random.uniform(0, 2*np.pi) for x in range(num_particles)]
		self.yaw_offset = 0

		# are we in the middle of an update?
		self.updating = False
		
	def __del__(self):
		pass

	def addPosEstimate(self, xyz, t):
		# updating mutex
		if self.updating:
			return

		# append to history
		self.posHistory.append( (xyz, t) )

		# prune old history
		while (t - self.posHistory[0][1]) > WINDOW_PERIOD_SEC and len(self.posHistory) > 0:
			self.posHistory.pop(0)

	def addPose(self, RPY, thrust, t):
		# updating mutex
		if self.updating:
			return

		# append to history
		self.rpyHistory.append( (RPY, thrust, t) )

		# prune old history
		while (t - self.rpyHistory[0][2]) > WINDOW_PERIOD_SEC and len(self.rpyHistory) > 0:
			self.rpyHistory.pop(0)


	def setInnovation(self, innov):
		self.innovation = innov

	def _thrustToForce(self, thrust):
		# just an arbitrary factor for now to estimate m/s travel at nominal thrust
		mps_hover = 0.30
		hover = 35000.0/65535.0
		return (thrust/65535.0)*(mps/hover)

	def _estimateCfPath(self):
		if len(self.rpyHistory) < 2:
			return None

		# start at origin
		path = [np.array([0,0])]

		for idx in range(1,len(self.rpyHistory)):
			data   = self.rpyHistory[idx]
			[roll, pitch, yaw] = data[0][0]
			thrust = self._thrustToForce(data[0][1])
			t      = data[0][2]
			dt = t - self.rpyHistory[idx-1][2]
			# calculate new point
			pold = path[idx-1]
			dpitch = thrust*dt*np.sin(pitch)
			droll = thrust*dt*np.sin(roll)
			dx = np.cos(yaw)*dpitch + np.sin(yaw)*droll
			dy = np.sin(yaw)*dpitch + np.cos(yaw)*droll
			pnew = pold + (dx,dy)
			path.append(pnew)

		# return last point
		return path[-1]

	def _estimateNtbPath(self):
		return self.posHistory[-1] - self.posHistory[0]

	def _getDistance(self, xyz1, xyz2):
		# this should really be angular distance, but euclidean shall suffice for now
		return np.norm( xyz1 - xyz2 )

	def update(self):
		# lock aggregator mutex
		self.updating = True

		# if we don't have enough new measurements, there's nothing to do here
		if len(self.posHistory) < 2 or len(self.rpyHistory < 2):
			return

		# get estimated cf-frame path
		path_cf = self._estimateCfPath()
		self.rpyHistory.clear()

		# get estimated NTB-frame path
		path_ntb = self._estimateNtbPath()
		self.posHistory.clear()

		# calculate likelihood for all particles
		for p in range(self.Np):
			# add innovation noise
			self.yaw_particles[p] += self.innovationNoise()

			# constrain particle to [0, 2pi]
			self.yaw_particles[p] = self.yaw_particles[p] % (2*np.pi)

			# calculate new likelihood based on measurements since last update
			L = self.getLikelihood(path_cf, path_ntb)

			# append to likelihood array
			self.L_particles[p] = L

		# In case we got to a bad state where all particles have zero probability, restart PF
		L_sum = np.sum(self.L_particles)

		if L_sum == 0:
			print('Warning: PF in bad state, restarting')
			self.aggregator.clear()
			return

		# normalize likelihoods to sum to 1. 
		self.L_particles /= L_sum

		# resample particles based on normalized probability density function
		new_particle_idxs = np.random.choice(range(self.Np), self.Np, True, p=self.L_particles)
		self.yaw_particles = self.yaw_particles[new_particle_idxs]

		# get new position estimate via particle centroid (mean on axis 0)
		self.yaw_offset = np.mean(self.yaw_particles, 0)

		# done updating
		self.updating = False

		return self.yaw_offset

	def getLikelihood(self, path_cf, path_ntb):
		print("ntb path = ", path_ntb)
		print("cf path = ", path_cf)
		dist = self._getDistance(path_cf, path_ntb)
		# basic likelihood written below only takes around 3.5 us
		return (1.0/(self.meas_noise*np.sqrt(2.0*np.pi))) * np.exp( (-(dist)**2)/(2.0*self.meas_noise**2))

	def getEstimate(self):
		return self.yaw_offset

	def innovationNoise(self):
		return np.random.normal(0 ,self.innovation)


# test particle filter standalone
if __name__ == '__main__':
	pf = ParticleFilterYaw(100, 10.0*np.pi/180.0)
	print(pf.getEstimate())
	pass