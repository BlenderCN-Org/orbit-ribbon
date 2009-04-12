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
	
	def __init__(self, pos):
		geom = ode.GeomCapsule(app.dyn_space, 0.25, 2.0)
		geom.coll_props = collision.Props()
		super(Ring, self).__init__(pos = pos, body = sphere_body(2000, 1), geom = geom)
	
	def step(self):
		pass
	
	def indraw(self):
		pass
