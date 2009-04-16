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
		space = ode.SimpleSpace(app.dyn_space)
		for i in xrange(self.STEPS):
			subgeom = ode.GeomSphere(None, radius = self.INNER_RAD)
			subgeomT = ode.GeomTransform(space)
			subgeomT.setGeom(subgeom)
			subgeomT.setInfo(1)
			subgeomT.coll_props = collision.Props()
			s = math.sin(2*math.pi*(i/self.STEPS))
			c = math.cos(2*math.pi*(i/self.STEPS))
			subgeom.setPosition((c*self.OUTER_RAD, s*self.OUTER_RAD, 0.0))
		
		super(Ring, self).__init__(pos = pos, body = None, geom = space)
		
		self._thruSound = resman.SoundClip("/usr/share/sounds/question.wav")
	
	def step(self):
		for e in app.events:
			if joy.procEvent(e) == (joy.BUTTON, joy.X, joy.DOWN):
				self._thruSound.snd.play()
	
	def indraw(self):
		glColor3f(*colors.red)
		glutSolidTorus(self.INNER_RAD, self.OUTER_RAD, 20, self.STEPS)
