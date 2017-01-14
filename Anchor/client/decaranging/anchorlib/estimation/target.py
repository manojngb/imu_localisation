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

# ===== TARGET CLASS (e.g. crazyflie) =====
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