import ode
from OpenGL.GL import *
from OpenGL.GLUT import *

import gameobj, colors
from geometry import *

class Cube(gameobj.GameObj):
	"""An object for testing; a simple cube.
	
	Data attributes:
	color - The color of the cube (defaults to red).
	scale - The size of the cube (defaults to 1.0).
	"""
	
	def __init__(self, pos, color = colors.red, scale = 1.0):
		super(Cube, self).__init__(pos)
		self.color = color
		self.scale = scale
	
	def indraw(self):
		glColor3fv(self.color)
		glutSolidCube(self.scale)
