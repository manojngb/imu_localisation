# ===== IMPORTS =====

class Target:
	def __init__(self, addr):
		self.addr = addr
		self.estimator = None

	def __del__(self):
		del self.estimator

	def getAddress(self):
		return self.addr

	def setEstimator(self, estimator):
		self.estimator = estimator

	def addRangeMeasurement(self, uid, xyz, dist, variance):
		if self.estimator != None:
			self.estimator.addRangeMeasurement(uid, xyz, dist, variance)

	def updateEstimate(self):
		if self.estimator != None:
			self.estimator.update()

	def getEstimate(self):
		if self.estimator != None:
			return self.estimator.getEstimate()
		else:
			return None