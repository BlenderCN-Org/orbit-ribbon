from __future__ import division

import ode, math, random

import app, collision, resman, billboard
from geometry import *
from util import *
from gl import *

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

# Ambient light settings
AMB_LIGHT_DIST = 1e10 # Distance to the ambient light (not too important, as there's no attenuation)
AMB_LIGHT_DIFFUSE = (0.15, 0.15, 0.15, 1.0) # Diffuse color of the ambient light
		
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
		

# FIXME - Consider using depth testing for sky objects, then clearing the depth buffer

class SkyStuff:
	"""Handles and draws the objects that are visible far out in the sky; Voy and T3, Gold, the Smoke Ring, far ponds and clouds and plants, etc.

	You need to call both draw_geometry and draw_billboards once each frame, in that order, before drawing anything else.
	Furthermore, you must set the far clipping plane to app.SKY_CLIP_DIST away before calling these methods.
	
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
		
		self._voy_tex = resman.Texture("voy.png")
		self._t3_tex = resman.Texture("t3.png")
		self._gold_tex = resman.Texture("gold.png")
		self._jungle_tex = resman.Texture("jungle.png")
		
		self._billboardList = billboard.BillboardList()
		t3_pos = self._t3_pos()
		self._billboardList.append(billboard.Billboard(t3_pos,               self._t3_tex,   T3_RADIUS*2))
		self._billboardList.append(billboard.Billboard(Point(GOLD_DIST,0,0), self._gold_tex, GOLD_RADIUS*2))
		self._billboardList.append(billboard.Billboard(Point(0,0,0),         self._voy_tex,  VOY_RADIUS*2))
		for p in JUNGLE_POSITIONS:
			# Draw jungles around the Smoke Ring in various randomly generated positions
			self._billboardList.append(billboard.Billboard(p, self._jungle_tex, 20000))
	
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
	
	def _t3_pos(self):	
		# FIXME - Verify that this makes T3 spin in the correct direction as angle increases
		return Point(math.sin(rev2rad(self.t3_angle))*T3_DIST, 0, math.cos(rev2rad(self.t3_angle))*T3_DIST)
	
	def draw_geometry(self):
		"""Draws the stars and the atmosphere, and sets up lighting for T3 and Voy.
		
		You must set the far clipping plane to app.SKY_CLIP_DIST away before calling, and disable depth testing.
		"""
		
		# Position and rotate ourselves; our origin (at Voy) is not the gameplay coordinate system origin
		glPushMatrix()
		self._applySkyMatrix()
		
		### The sky and distant stars (if visible)
		cam = app.player_camera.get_position()
		ring_dist = self.get_dist_from_ring(cam)
		sky_ratio = 1.0 - min(1.0, ring_dist/(SMOKE_RING_RADIUS*60)) # At 1.0, full sky color. At 0.0, we're in pitch-black space.
		glPushMatrix()
		if sky_ratio > 0.001:
			# Draw a sphere for the sky
			full_color = (0.6, 0.6, 1.0) # The color of the sky at the densest part of the smoke ring
			glColor3f(full_color[0]*sky_ratio, full_color[1]*sky_ratio, full_color[2]*sky_ratio)
			glutSolidSphere(1e15, 15, 15)
		if sky_ratio < 0.5:
			# Draw stars
			cachingGlEnable(GL_POINT_SMOOTH)
			for n, sublist in enumerate(STAR_LISTS):
				size = 0.5 + (n+1)*1.5/len(STAR_LISTS) - sky_ratio
				glColor3f(size/2 + 0.5, size/2 + 0.5, size/2 + 0.5)
				glPointSize(size + 1.0)
				glBegin(GL_POINTS)
				for pt in sublist:
					glVertex3f(*pt)
				glEnd()
			cachingGlDisable(GL_POINT_SMOOTH)
		glPopMatrix()
		
		### The Smoke Ring and the gas torus
		cachingGlEnable(GL_CULL_FACE)
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
		cachingGlDisable(GL_CULL_FACE) # FIXME - Should have back face culling always enabled to hide camera mishaps and maybe boost performance a little
		
		# Set up lighting parameters for the two stars of the Smoke Ring system
		# Since lighting is disabled at this time, this does not affect the drawing of the sky objects themselves
		
		# T3
		t3_pos = self._t3_pos()
		glLightfv(GL_LIGHT1, GL_POSITION, (t3_pos[0], t3_pos[1], t3_pos[2], 1.0))
		glLightfv(GL_LIGHT1, GL_DIFFUSE, (1.0, 1.0, 1.0, 1.0))
		cachingGlEnable(GL_LIGHT1)
		
		# Voy
		# FIXME - Set up lighting for Voy
		
		# Ambient lighting (so that areas not lit by T3 or Voy aren't completely dark)
		part_ald = math.sqrt(2)*AMB_LIGHT_DIST
		for lightcons, pos in (
			(GL_LIGHT3, (0, AMB_LIGHT_DIST, 0)),
			(GL_LIGHT4, (part_ald, -part_ald, 0)),
			(GL_LIGHT5, (-0.5*part_ald, -part_ald, 0.866*part_ald)),
			(GL_LIGHT6, (-0.5*part_ald, -part_ald, -0.866*part_ald)),
		):
			glLightfv(lightcons, GL_POSITION, (pos[0], pos[1], pos[2], 1.0))
			glLightfv(lightcons, GL_DIFFUSE, AMB_LIGHT_DIFFUSE)
			cachingGlEnable(lightcons)
		
		glPopMatrix()
	
	def draw_billboards(self):
		"""Draws the billboards which represent distant sky objects.
		
		You must set the far clipping plane to app.SKY_CLIP_DIST away before calling, and enable depth testing, and enable TEXTURE_2D in GL_REPLACE mode.
		"""
		### Figure out where the camera is and how to get there, and move to the sky coordinate system
		cam = app.player_camera.get_position()
		localCamPos = self.to_sky_coords(cam)
		glPushMatrix()
		self._applySkyMatrix()
		self._billboardList.draw(localCamPos)
		glPopMatrix()
