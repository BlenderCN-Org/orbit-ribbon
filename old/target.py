from __future__ import division
import ode, math

import app, gameobj, collision, resman, avatar
from geometry import *
from util import *

class Ring(gameobj.GameObj):
	"""A ring that the player is intended to pass through.
	
	Data attributes:
	passedThru - Becomes True once the player has passed through the ring.
	"""
	OUTER_RAD = 7.0
	INNER_RAD = 1.0
	STEPS = 35
	
	def __init__(self, oreman, pos, rot):
		self._oreman = oreman
		self._oremesh = oreman.meshes["LIBTargetRing"]

		subspace = ode.SimpleSpace(app.static_space)
		
		# Logical geom for detecting when something has passed through the ring
		# FIXME: PyODE's GeomCylinder doesn't seem to work, but that's really what I need to use here.
		self._logicGeom = ode.GeomCapsule(subspace, radius = self.OUTER_RAD-self.INNER_RAD/2, length = self.INNER_RAD*1.5)
		self._logicGeom.coll_props = collision.Props(intersec_push = False)
		
		# Collision geom for the substance of the ring
		coll_geom = ode.GeomTriMesh(self._oremesh.trimesh_data, subspace)
		coll_geom.coll_props = collision.Props()
		
		super(Ring, self).__init__(pos = pos, rot = rot, body = None, geom = subspace)
		
		self.passedThru = False # True once the player has passed through the ring
		self._thruSound = resman.SoundClip("/usr/share/sounds/question.wav")
	
	def step(self):
		if self.passedThru is False and id(self._logicGeom) in app.collisions:
			for coll in app.collisions[id(self._logicGeom)]:
				if isinstance(coll.geom.gameobj, avatar.Avatar):
					self.passedThru = True
					self._thruSound.snd.play()
	
	def indraw(self):
		self._oremesh.draw_gl()
