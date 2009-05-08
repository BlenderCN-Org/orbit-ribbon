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
	game_x_tilt - A value in [0.0,1.0) indicating how tilted the Smoke Ring appears along the X axis from the player's perspective. Applied first.
	game_z_tilt - A value in [0.0,1.0) indicating how tilted the Smoke Ring appears along the Z axis from the player's perspective. Applied second.
	t3_angle - A value in [0.0,1.0) that indicates which part of the torus T3 is closest to.
	"""
	
	def __init__(self, game_angle = 0.0, game_y_offset = 0.0, game_d_offset = 0.0, game_x_tilt = 0.0, game_z_tilt = 0.0, t3_angle = 0.3):
		self.game_angle = game_angle
		self.game_y_offset = game_y_offset
		self.game_d_offset = game_d_offset
		self.game_x_tilt = game_x_tilt
		self.game_z_tilt = game_z_tilt
		self.t3_angle = t3_angle
		
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
	
	def draw(self):
		# Position and rotate ourselves; our origin (at Voy) is not the gameplay coordinate system origin
		glPushMatrix()
		glRotatef(rev2deg(self.game_x_tilt), 1, 0, 0) # Apply X tilt
		glRotatef(rev2deg(self.game_z_tilt), 0, 0, 1) # Apply Z tilt
		glTranslatef(0.0, -self.game_y_offset, GOLD_DIST + self.game_d_offset) # Move out to Voy
		glRotatef(rev2deg(self.game_angle), 0, 1, 0) # Rotate around the Ring until we get to our gameplay spot
		
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
		
		billboard_queue = [] # List of (tex/color, x, y, z, scale) tuples for ortho 2d drawing
		def enqueue_billboard(pos, tex, color, scale):
			winPos = gluProject(*pos)
			if winPos[2] > 1.0:
				return
			s = scale*(1-winPos[2])*20000 # FIXME : Why do I need this huge fudge coefficient? I know from experiment that I don't need SKY_CLIP_DIST...
			# FIXME One way to test above is to bring back quad drawing, and make billboards alpha to do testing...
			if winPos[0] + s/2 < 0 or winPos[0] - s/2 > app.winsize[0] or winPos[1] + s/2 < 0 or winPos[1] - s/2 > app.winsize[1]:
				return
			if s < 1.5:
				# Add it as a colored point billboard
				billboard_queue.append((color, winPos[0], winPos[1], winPos[2], s))
			elif s > 0.3:
				# Add it as a textured billboard
				billboard_queue.append((tex, winPos[0], winPos[1], winPos[2], s))
		
		# Add billboards for major objects
		enqueue_billboard(t3_pos,               self._t3_tex,   colors.yellow, T3_RADIUS*2)    # T3
		enqueue_billboard(Point(0,0,GOLD_DIST), self._gold_tex, colors.gray,   GOLD_RADIUS*2)  # Gold
		enqueue_billboard(Point(0,0,0),         self._voy_tex,  colors.blue,   VOY_RADIUS*2)   # Voy
		
		# Add billboards for jungles around the Smoke Ring in various positions
		for p in self._jungle_positions:
			enqueue_billboard(p, self._jungle_tex, (0.0, 0.5, 0.0), 20000)
		
		## Angle to rotate billboards: the angle between the camera's up vector and the north-south axis
		# Create our north_up vector by starting with positive y axis vector, then applying X and Z tilt
		north_up = Point(0, 1, 0)
		# FIXME Implement geometry.Point.rotate, then use it to rotate north_up
		# Find the angle between in radians using dot product
		billboard_ang = rev2deg(rad2rev(math.acos(app.camera_up.dot_prod(north_up)/app.camera_up.mag())))
		
		# Draw everything in the billboard queue in back-to-front order
		billboard_queue.sort(cmp = lambda x, y: cmp(y[3], x[3]))
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE)
		glMatrixMode(GL_PROJECTION)
		glLoadIdentity()
		gluOrtho2D(0, app.winsize[0], 0, app.winsize[1])
		glMatrixMode(GL_MODELVIEW)
		glLoadIdentity()
		for tex_or_color, x, y, z, s in billboard_queue:
			if isinstance(tex_or_color, resman.Texture):
				glEnable(GL_TEXTURE_2D)	
				glBindTexture(GL_TEXTURE_2D, tex_or_color.glname)
				s2 = s/2
				glPushMatrix()
				glTranslatef(x, y, 0)
				glRotatef(billboard_ang, 0, 0, 1)
				glBegin(GL_QUADS)
				glTexCoord2f(0.0, 0.0)
				glVertex3f(-s2, -s2, 0)
				glTexCoord2f(1.0, 0.0)
				glVertex3f(+s2, -s2, 0)
				glTexCoord2f(1.0, 1.0)
				glVertex3f(+s2, +s2, 0)
				glTexCoord2f(0.0, 1.0)
				glVertex3f(-s2, +s2, 0)
				glEnd()
				glPopMatrix()
				glDisable(GL_TEXTURE_2D)
			else:
				glPointSize(s)
				glColor3fv(tex_or_color)
				glBegin(GL_POINTS)
				glVertex3f(x, y, 0)
				glEnd()
				
		glPopMatrix()
