from __future__ import division
import ode
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
	
	def __init__(self, pos):
		#geom = ode.SimpleSpace()
		#geom.coll_props = collision.Props()
		#super(Ring, self).__init__(pos = pos, body = sphere_body(2000, 1), geom = geom)
		super(Ring, self).__init__(pos = pos, body = sphere_body(2000, 1))
	
	def step(self):
		pass
	
	def indraw(self):
		glColor3f(*colors.red)
		glutSolidTorus(self.INNER_RAD, self.OUTER_RAD, 20, 25)
