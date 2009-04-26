import ode
from OpenGL.GL import *
from OpenGL.GLUT import *

import app, gameobj, colors, collision, resman
from geometry import *
from util import *

class SkyStuff(gameobj.GameObj):
	"""The objects that are visible far out in the sky; Voy and T3, Gold, the Smoke Ring, stars, etc.
	
	The origin of this object is the location of Voy. The Sun is on the local x axis.
	
	It is important to draw this object each frame before anything else. Furthermore, before drawing, one must
	disable depth testing and the depth buffer, and set the far clipping plane to at least 3e11 away.
	
	No dynamics or geometry are involved with this object, since everything is so far away that there's
	no reason for the player to ever interact with it.
	
	Data attributes:
	day_elapsed - A value in [0.0,1.0) that indicates how much of the day has passed.
		At day_elapsed = 0.0, Gold is on the local x axis. Each day, Gold makes one orbit around Voy.
	"""
	
	# Distances in meters from Voy to other sky objects, and sizes of various objects
	VOY_RADIUS = 2e4 # From book
	T3_DIST = 2.5e11 # From book
	T3_RADIUS = 8.4e8 # Our Sun's radius times about 1.2
	TORUS_OUTSIDE_DIST = 1e9 # From book
	TORUS_INSIDE_DIST = 1e7 # Guessed
	TORUS_RADIUS = TORUS_OUTSIDE_DIST - TORUS_INSIDE_DIST # Assuming a circular torus, which probably isn't quite right
	GOLD_DIST = 2.6e7 # From book; also, assuming that Gold is in precise middle of Smoke Ring
	GOLD_RADIUS = 1e7 # Guessed; includes the storm around Gold
	SMOKE_RING_RADIUS = 1.4e7 # Calculated; book says that SR has volume of 1e14 cubic km, and assumed a circular torus...
	SMOKE_RING_INSIDE_DIST = GOLD_DIST - SMOKE_RING_WIDTH
	SMOKE_RING_OUTSIDE_DIST = GOLD_DIST + SMOKE_RING_WIDTH
	
	def __init__(self, pos, day_elapsed = 0.0):
		super(SkyStuff, self).__init__(pos = pos)
		self.day_elapsed = day_elapsed
	
	def indraw(self):
		glColor3fv(self.color)
		glutSolidCube(self._scale)
