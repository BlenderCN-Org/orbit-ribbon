import ode
from OpenGL.GL import *
from OpenGL.GLUT import *

import app, gameobj, colors, collision, resman
from geometry import *
from util import *
	
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
SMOKE_RING_INSIDE_DIST = GOLD_DIST - SMOKE_RING_RADIUS
SMOKE_RING_OUTSIDE_DIST = GOLD_DIST + SMOKE_RING_RADIUS

class SkyStuff(gameobj.GameObj):
	"""The objects that are visible far out in the sky; Voy and T3, Gold, the Smoke Ring, far ponds and clouds and plants, etc.
	
	The origin of this object is the location of Voy. Gold is on the local x axis.
	
	It is important to draw this object each frame before anything else. Furthermore, before drawing, one must
	disable depth testing, and set the far clipping plane to at least 1e12 away.
	
	No dynamics or geometry are involved with this object, since everything is so far away that there's
	no reason for the player to ever interact with it.
	
	Data attributes:
	day_elapsed - A value in [0.0,1.0) that indicates how much of the day has passed.
		At day_elapsed = 0.0, T3 is on the far side of Voy. Each day, T3 makes one apparent orbit around Voy.
	"""
	
	def __init__(self, pos, day_elapsed = 0.0):
		super(SkyStuff, self).__init__(pos = pos)
		self.day_elapsed = day_elapsed
		self._voy_tex = resman.Texture("voy.png")
		self._t3_tex = resman.Texture("t3.png")
		self._gold_tex = resman.Texture("gold.png")
	
	def indraw(self):
		glEnable(GL_TEXTURE_2D)
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE)
		
		def draw_billboard(rot, pos, tex, width, height):
			glBindTexture(GL_TEXTURE_2D, tex.glname)
			glPushMatrix()
			glRotatef(*rot)
			glTranslatef(*pos)
			# FIXME - Rotate to orient towards camera
			glBegin(GL_QUADS)
			glTexCoord2f(0.0, 0.0)
			glVertex3f(-width/2, -height/2, 0)
			glTexCoord2f(1.0, 0.0)
			glVertex3f( width/2, -height/2, 0)
			glTexCoord2f(1.0, 1.0)
			glVertex3f( width/2,  height/2, 0)
			glTexCoord2f(0.0, 1.0)
			glVertex3f(-width/2,  height/2, 0)
			glEnd()
			glPopMatrix()
		
		# FIXME - Sort from farthest to closest; depending on position and time of day, Gold or Voy may be closer
		draw_billboard((self.day_elapsed*360,0,1,0), (0,0,T3_DIST),   self._t3_tex,   T3_RADIUS*2,   T3_RADIUS*2)
		draw_billboard((0,0,0,0),                    (0,0,0),         self._voy_tex,  VOY_RADIUS*2,  VOY_RADIUS*2)
		draw_billboard((0,0,0,0),                    (GOLD_DIST,0,0), self._gold_tex, GOLD_RADIUS*2, GOLD_RADIUS*2)
		
		glDisable(GL_TEXTURE_2D)
