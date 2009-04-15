from __future__ import division
import ode, math
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

import app, gameobj, colors, collision, joy
from geometry import *
from util import *

class Ring(gameobj.GameObj):
	"""A ring that the player is intended to pass through."""
	OUTER_RAD = 7.0
	INNER_RAD = 1.0
	STEPS = 25
	
	def __init__(self, pos):
		space = ode.SimpleSpace(app.dyn_space)
		for i in xrange(self.STEPS):
			subgeom = ode.GeomCapsule(None, radius = self.INNER_RAD*10, length = 2*math.pi*self.OUTER_RAD/self.STEPS)
			subgeomT = ode.GeomTransform(space)
			subgeomT.setGeom(subgeom)
			s = math.sin(2*math.pi*(i/self.STEPS))
			c = math.cos(2*math.pi*(i/self.STEPS))
			#subgeom.setPosition((c*(self.OUTER_RAD-self.INNER_RAD), s*(self.OUTER_RAD-self.INNER_RAD), 0.0))
			#subgeom.setRotation() # Need to figure out what goes here
		
		super(Ring, self).__init__(pos = pos, body = sphere_body(2000, 1), geom = space)
	
	def step(self):
		pass
	
	def indraw(self):
		glColor3f(*colors.red)
		glutSolidTorus(self.INNER_RAD, self.OUTER_RAD, 20, self.STEPS)
