from __future__ import division

import ode, math, random
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

import app, colors, collision, resman
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
SMOKE_RING_RADIUS = 8e4 # Calculations yielded 1.4e7, but that looked terrible, so this is made up. FIXME This is far too narrow to hold jungles. 
SMOKE_RING_INSIDE_DIST = GOLD_DIST - SMOKE_RING_RADIUS
SMOKE_RING_OUTSIDE_DIST = GOLD_DIST + SMOKE_RING_RADIUS
		
# Create some random jungle positions
random.seed(2)
JUNGLE_POSITIONS = []
for i in range(150):
	ang = random.random()*2*math.pi
	r_offset = random.random()*SMOKE_RING_RADIUS - SMOKE_RING_RADIUS/2
	y_offset = random.random()*SMOKE_RING_RADIUS - SMOKE_RING_RADIUS/2
	p = Point(math.cos(ang)*(GOLD_DIST + r_offset), y_offset, math.sin(ang)*(GOLD_DIST + r_offset))
	JUNGLE_POSITIONS.append(p)

# Create some random star positions
random.seed(86)
STAR_LISTS = [] # A list of lists of Points, each sub-list delimiting an increasingly bright star group
ranval = lambda: (random.random()*2 - 1)*1e15
for i in range(50):
	sublist = []
	for j in range(100):
		sublist.append(Point(ranval(), ranval(), ranval()))
	STAR_LISTS.append(sublist)
		
VOY_TEX = resman.Texture("voy.png")
T3_TEX = resman.Texture("t3.png")
GOLD_TEX = resman.Texture("gold.png")
JUNGLE_TEX = resman.Texture("jungle.png")

# FIXME - Consider using depth testing for sky objects, then clearing the depth buffer

class SkyStuff:
	"""Handles and draws the objects that are visible far out in the sky; Voy and T3, Gold, the Smoke Ring, far ponds and clouds and plants, etc.
	
	It is important to draw this object each frame before anything else. Also, before drawing, you must
	disable depth testing and lighting, and set the far clipping plane to app.SKY_CLIP_DIST away.
	
	The 'angle' values below determine position around the Smoke Ring. Gold is at angle 0.5.
	
	Data attributes:
	game_angle - A value in [0.0,1.0) that indicates which part of the torus the origin of the gameplay coordinate system is.
	game_y_offset - A value indicating how far north or south from the Smoke Ring's middle the game coord system is at.
	game_d_offset - A value indicating how far in or out from the Smoke Ring's middle the game coord system is at.
	game_tilt - A 4-tuple for glRotatef indicating how the Smoke Ring should be rotated relative to the game world. Y-axis-value must be zero.
	t3_angle - A value in [0.0,1.0) that indicates which part of the torus T3 is closest to.
	"""
	
	def __init__(self, game_angle = 0.0, game_y_offset = 0.0, game_d_offset = 0.0, game_tilt = (0, 0, 0, 0), t3_angle = 0.3):
		self.game_angle = game_angle
		self.game_y_offset = game_y_offset
		self.game_d_offset = game_d_offset
		self.game_tilt = game_tilt
		self.t3_angle = t3_angle		
	
	def _applySkyMatrix(self):
		# TODO - Perhaps could be optimized by caching, since the matrix generated will not change as long as sky values don't change
		glRotatef(*self.game_tilt) # Apply tilt
		glTranslatef(0.0, -self.game_y_offset, GOLD_DIST + self.game_d_offset) # Move out to Voy
		glRotatef(rev2deg(self.game_angle), 0, 1, 0) # Rotate the Ring around Voy
	
	def _getSkyMatrix(self):
		glPushMatrix()
		glLoadIdentity()
		self._applySkyMatrix()
		r = glGetFloat(GL_MODELVIEW_MATRIX)
		glPopMatrix()
		return r
	
	def get_game_origin(self):
		"""Returns the position of the game origin in sky coordinates."""
		d = GOLD_DIST + self.game_d_offset
		return -Point(d*math.sin(rev2rad(self.game_angle)), self.game_y_offset, d*math.cos(rev2rad(self.game_angle)))
	
	def to_sky_coords(self, pt):
		"""Given a point in game coordinates, returns that point in sky coordinates under the current settings."""
		# Offset from the game origin to the target in sky coordinates
		skyMatrix = self._getSkyMatrix()
		localOffset = applyTransverseMatrix(pt, skyMatrix)
		return self.get_game_origin() + localOffset
	
	def get_voy_pos(self):
		"""Returns a Point() with the current location of Voy in gameplay coordinates."""
		# FIXME: This will only work when game_tilt is zero, must fix
		return Point(0, GOLD_DIST, 0)
	
	def get_starboard_vec(self):
		"""Returns a Point() with the starboard direction, the way that the Blue Ghost points, in gameplay coordinates."""
		# FIXME: This will only work when game_tilt is zero, must fix
		return Point(0, 1, 0)
	
	def get_dist_from_ring(self, pt):
		"""Given a Point in gameplay coordinates, returns its distance from the densest part of the Smoke Ring."""
		# This isn't entirely correct for getting distance from a circle, but it's close enough
		localPt = self.to_sky_coords(pt)
		xDist = abs(localPt.dist_to(Point(0,0,0)) - GOLD_DIST)
		yDist = localPt[1]
		return math.sqrt(xDist**2 + yDist**2)
	
	def draw(self):
		"""Draws everything in the sky, and saves the gameplay origin's projected screen position to the projectedPos attribute."""
		### Figure out where the camera is and how to get there, and move to the sky coordinate system
		cam = Point(*(app.player_camera.get_camvals()[0:3]))
		localCamPos = self.to_sky_coords(cam)
		
		# Position and rotate ourselves; our origin (at Voy) is not the gameplay coordinate system origin
		glPushMatrix()
		self._applySkyMatrix()
		
		### The sky and distant stars (if visible)
		ring_dist = self.get_dist_from_ring(cam)
		sky_ratio = 1.0 - min(1.0, ring_dist/(SMOKE_RING_RADIUS*10)) # At 1.0, full sky color. At 0.0, we're in pitch-black space.
		glPushMatrix()
		if sky_ratio > 0.001:
			# Draw a sphere for the sky
			full_color = (0.6, 0.6, 1.0) # The color of the sky at the densest part of the smoke ring
			glColor3f(full_color[0]*sky_ratio, full_color[1]*sky_ratio, full_color[2]*sky_ratio)
			glutSolidSphere(1e15, 15, 15)
		if sky_ratio < 0.5:
			# Draw stars
			glEnable(GL_POINT_SMOOTH)
			for n, sublist in enumerate(STAR_LISTS):
				size = 0.5 + (n+1)*1.5/len(STAR_LISTS)
				glColor3f(size/2 + 0.5, size/2 + 0.5, size/2 + 0.5)
				glPointSize(size + 1.0)
				glBegin(GL_POINTS)
				for pt in sublist:
					glVertex3f(*pt)
				glEnd()
			glDisable(GL_POINT_SMOOTH)
		glPopMatrix()
		
		### The Smoke Ring and the gas torus
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
		
		# FIXME - Verify that this makes T3 spin in the correct direction as angle increases
		t3_pos = Point(math.sin(rev2rad(self.t3_angle))*T3_DIST, 0, math.cos(rev2rad(self.t3_angle))*T3_DIST)
		
		# Set up lighting parameters for the two stars of the Smoke Ring system
		# Since lighting is disabled at this time, this does not affect the drawing of the sky objects themselves
		
		# T3
		glLightfv(GL_LIGHT1, GL_POSITION, (t3_pos[0], t3_pos[1], t3_pos[2], 1.0))
		glLightfv(GL_LIGHT1, GL_AMBIENT, (0.7, 0.7, 0.7, 1.0))
		glLightfv(GL_LIGHT1, GL_DIFFUSE, (1.0, 1.0, 1.0, 1.0))
		glEnable(GL_LIGHT1)
		
		# Voy
		# FIXME - Set up lighting for Voy
		
		def draw_billboard(pos, tex, width, height):
			# Vector from the billboard to the game origin
			camVec = -pos + localCamPos
			
			# If it's too far away compared to its size, don't bother with it
			# FIXME Come up with something meaningful here, not just this fudge value of 15
			if width*height/camVec.mag() < 15:
				return
			
			glBindTexture(GL_TEXTURE_2D, tex.glname)
			glPushMatrix()
			glTranslatef(*pos)
			
			# Rotate the billboard so that it faces the camera
			glRotatef(rev2deg(rad2rev(math.atan2(camVec[0], camVec[2]))), 0, 1, 0) # Rotate around y-axis...
			glRotatef(-rev2deg(rad2rev(math.atan2(camVec[1], math.sqrt(camVec[0]**2 + camVec[2]**2)))), 1, 0, 0) # Then tilt up/down on x-axis
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
		
		glEnable(GL_TEXTURE_2D)
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE)
		
		draw_billboard(t3_pos,               T3_TEX,   T3_RADIUS*2,   T3_RADIUS*2)    # T3
		draw_billboard(Point(GOLD_DIST,0,0), GOLD_TEX, GOLD_RADIUS*2, GOLD_RADIUS*2)  # Gold
		draw_billboard(Point(0,0,0),         VOY_TEX,  VOY_RADIUS*2,  VOY_RADIUS*2)   # Voy
		
		# Draw jungles around the Smoke Ring in various positions
		for p in JUNGLE_POSITIONS:
			draw_billboard(p, JUNGLE_TEX, 20000, 20000)
		
		glDisable(GL_TEXTURE_2D)
		glPopMatrix()
