import ode
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

import app, gameobj, colors, collision, joy, resman
from geometry import *
from util import *

MAX_STRAFE = 10.0 # In N/s
MAX_ACCEL = 35.0 # In N/s

class Avatar(gameobj.GameObj):
	"""The player character."""
	
	def __init__(self, pos):
		geom = ode.GeomCapsule(app.dyn_space, 0.25, 2.0) # 2.5 total length: a 2.0-long cylinder, and two 0.25-radius caps
		geom.coll_props = collision.Props()
		super(Avatar, self).__init__(pos = pos, body = sphere_body(1, 0.2), geom = geom)
		self._quad = gluNewQuadric()
		gluQuadricTexture(self._quad, GLU_TRUE)
		self._tex = resman.Texture("lava.png")
	
	def __del__(self):
		gluDeleteQuadric(self._quad)
	
	def step(self):
		if app.axes[joy.LX] != 0.0:
			self.body.addRelForce((-app.axes[joy.LX]*(MAX_STRAFE/app.maxfps), 0, 0))
		if app.axes[joy.LY] != 0.0:
			self.body.addRelForce((0, -app.axes[joy.LY]*(MAX_STRAFE/app.maxfps), 0))
		if app.axes[joy.L2] != 0.0:
			self.body.addRelForce((0, 0, -app.axes[joy.L2]*(MAX_ACCEL/app.maxfps)))
		elif app.axes[joy.R2] != 0.0:
			self.body.addRelForce((0, 0, app.axes[joy.R2]*(MAX_ACCEL/app.maxfps)))
					
		app.camera = self.pos.__copy__(); app.camera[0] += 0; app.camera[1] += 1; app.camera[2] -= 6
		app.camera_tgt = self.pos.__copy__()
	
	def indraw(self):
		# The cylinder body
		glTranslatef(0, 0, -1.0)
		glEnable(GL_TEXTURE_2D)
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE)
		glBindTexture(GL_TEXTURE_2D, self._tex.glname)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
		gluCylinder(self._quad, 0.25, 0.15, 2.0, 30, 10)
		glDisable(GL_TEXTURE_2D)
		
		# The "feet"
		glColor3f(*colors.cyan)
		glutSolidSphere(0.25, 15, 15)
		
		# Back up to the "head"
		glTranslatef(0, 0, 2.0)
		glColor3f(*colors.red)
		glutSolidSphere(0.20, 15, 15)
