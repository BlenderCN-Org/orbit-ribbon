import ode
from OpenGL.GL import *
from OpenGL.GLUT import *

import app, gameobj, colors, collision
from geometry import *
from util import *

class Cube(gameobj.GameObj):
	"""An object for testing; a simple cube.
	
	Data attributes:
	color - The color of the cube (defaults to red).
	scale - The size of the cube (defaults to 1.0).
	"""
	
	def __init__(self, pos, color = colors.red, scale = 1.0):
		geom = ode.GeomBox(app.dyn_space, (scale, scale, scale))
		geom.coll_props = collision.Props()
		super(Cube, self).__init__(pos = pos, body = sphere_body(1, 0.375), geom = geom)
		self.color = color
		self.scale = scale
	
	def indraw(self):
		glColor3fv(self.color)
		glutSolidCube(self.scale)
