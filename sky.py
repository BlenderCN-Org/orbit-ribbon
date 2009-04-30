import ode, math, random
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

import app, gameobj, colors, collision, resman
from geometry import *
from util import *
	
# Distances in meters from Voy to other sky objects, and sizes of various objects
VOY_RADIUS = 2e4 # From book
T3_DIST = 2.5e11 # From book
T3_RADIUS = 2.5e9 # Our Sun's radius times about 1.2 is 8.4e8, which is from book... but that looks too small
TORUS_OUTSIDE_DIST = 1e9 # From book
TORUS_INSIDE_DIST = 1e7 # Guessed
TORUS_RADIUS = TORUS_OUTSIDE_DIST - TORUS_INSIDE_DIST # Assuming a circular torus, which probably isn't quite right
GOLD_DIST = 2.6e7 # From book; also, assuming that Gold is in precise middle of Smoke Ring
GOLD_RADIUS = 5e5 # Guessed; includes the storm around Gold
SMOKE_RING_RADIUS = 8e4 # Calculations yielded 1.4e7, but that looked terrible, so this is made up 
SMOKE_RING_INSIDE_DIST = GOLD_DIST - SMOKE_RING_RADIUS
SMOKE_RING_OUTSIDE_DIST = GOLD_DIST + SMOKE_RING_RADIUS

# FIXME - Consider using depth testing for sky objects, then clearing the depth buffer

class SkyStuff(gameobj.GameObj):
	"""The objects that are visible far out in the sky; Voy and T3, Gold, the Smoke Ring, far ponds and clouds and plants, etc.
	
	The origin of this object is the location of Voy. Gold is on the local x axis.
	
	It is important to draw this object each frame before anything else. Also, before drawing, you must
	disable depth testing and lighting, and set the far clipping plane to at least 1e12 away.
	
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
		self._jungle_tex = resman.Texture("jungle.png")
		
		# Create some random jungle positions
		random.seed(2)
		self._jungle_positions = []
		for i in range(150):
			ang = random.random()*2*math.pi
			r_offset = random.random()*SMOKE_RING_RADIUS - SMOKE_RING_RADIUS/2
			y_offset = random.random()*SMOKE_RING_RADIUS - SMOKE_RING_RADIUS/2
			p = Point(math.cos(ang)*(GOLD_DIST + r_offset), y_offset, math.sin(ang)*(GOLD_DIST + r_offset))
			self._jungle_positions.append(p)
	
	def indraw(self):
		# The Smoke Ring and the gas torus
		glEnable(GL_CULL_FACE)
		glPushMatrix()
		glRotatef(90, 1, 0, 0)
		# Smoke ring itself
		glColor4f(1.0, 1.0, 1.0, 0.5)
		glutSolidTorus(SMOKE_RING_RADIUS, GOLD_DIST, 10, 50)
		# Outer "fuzz" from the smoke ring
		for i in range(3, 20, 3):
			glRotatef(2, 0, 0, 1)
			glColor4f(1.0, 1.0, 1.0, 0.08)
			glutSolidTorus(SMOKE_RING_RADIUS*i, GOLD_DIST, 10, 50)
		# The gas torus
		# FIXME - Rendering it effectively only tints background color. Should it do something else? Should I bother?
		#glColor4f(1.0, 1.0, 1.0, 0.05)
		#glutSolidTorus(TORUS_RADIUS, (TORUS_INSIDE_DIST+TORUS_OUTSIDE_DIST)/2, 20, 20)
		glPopMatrix()
		glDisable(GL_CULL_FACE)
		
		# FIXME - Verify that this makes T3 spin in the correct direction
		t3_pos = Point(math.sin(rev2rad(self.day_elapsed))*T3_DIST, 0, math.cos(rev2rad(self.day_elapsed))*T3_DIST)
		
		glEnable(GL_TEXTURE_2D)
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE)
		
		# Set up lighting parameters for the two stars of the Smoke Ring system
		# Since lighting is disabled at this time, this does not affect the drawing of the sky objects themselves
		
		# T3
		glPushMatrix()
		glTranslatef(*t3_pos)
		glLightfv(GL_LIGHT1, GL_POSITION, (0.0, 0.0, 0.0, 1.0))
		glPopMatrix()
		glLightfv(GL_LIGHT1, GL_AMBIENT, (0.7, 0.7, 0.7, 1.0))
		glLightfv(GL_LIGHT1, GL_DIFFUSE, (1.0, 1.0, 1.0, 1.0))
		glEnable(GL_LIGHT1)
		
		# Voy
		# FIXME - Set up lighting for Voy
		
		def draw_billboard(pos, tex, width, height):
			glBindTexture(GL_TEXTURE_2D, tex.glname)
			glPushMatrix()
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
		
		draw_billboard(t3_pos,               self._t3_tex,   T3_RADIUS*2,   T3_RADIUS*2)    # T3
		draw_billboard(Point(GOLD_DIST,0,0), self._gold_tex, GOLD_RADIUS*2, GOLD_RADIUS*2)  # Gold
		draw_billboard(Point(0,0,0),         self._voy_tex,  VOY_RADIUS*2,  VOY_RADIUS*2)   # Voy
		
		# Draw jungles around the Smoke Ring in various positions
		glRotatef(90, 0, 1, 0) # FIXME Stupid hack until billboards working properly
		for p in self._jungle_positions:
			draw_billboard(p, self._jungle_tex, 20000, 20000)
		
		glDisable(GL_TEXTURE_2D)
