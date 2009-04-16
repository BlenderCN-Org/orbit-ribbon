from __future__ import division
import ode, math
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

import app, gameobj, colors, collision, joy, resman
from geometry import *
from util import *

class Ring(gameobj.GameObj):
	"""A ring that the player is intended to pass through."""
	OUTER_RAD = 7.0
	INNER_RAD = 1.0
	STEPS = 35
	
	def __init__(self, pos):
		# A space for the geoms associated with this ring
		subspace = ode.SimpleSpace(app.static_space)
		
		# Collision geoms for the ring substance
		for i in xrange(self.STEPS):
			subgeomT = ode.GeomTransform(subspace)
			subgeomT.setInfo(1)
			subgeomT.coll_props = collision.Props()
			s = math.sin(2*math.pi*(i/self.STEPS))
			c = math.cos(2*math.pi*(i/self.STEPS))
			subgeom = ode.GeomSphere(space = None, radius = self.INNER_RAD)
			subgeom.setPosition((c*self.OUTER_RAD, s*self.OUTER_RAD, 0.0))
			subgeomT.setGeom(subgeom)
		
		# Logical geom for detecting when something has passed through the ring
		# FIXME: PyODE's GeomCylinder doesn't seem to work, but that's really what I need to use here.
		self._logicGeom = ode.GeomCapsule(subspace, radius = self.OUTER_RAD-self.INNER_RAD/2, length = self.INNER_RAD*1.5)
		self._logicGeom.coll_props = collision.Props(intersec_push = False)
		
		super(Ring, self).__init__(pos = pos, body = None, geom = subspace)
		
		self._passingThru = False # True while something is in the process of passing through the ring
		self._thruSound = resman.SoundClip("/usr/share/sounds/question.wav")
	
	def step(self):
		if id(self._logicGeom) in app.collisions:
			if self._passingThru is False:
				self._passingThru = True
				self._thruSound.snd.play()
		else:
			self._passingThru = False
	
	def indraw(self):
		glColor3f(*colors.red)
		glutSolidTorus(self.INNER_RAD, self.OUTER_RAD, 20, self.STEPS)
