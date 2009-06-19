from __future__ import division
import ode
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

import app, gameobj, colors, collision, joy, resman
from geometry import *
from util import *

# Maximum amount of force per second applied by various maneuvers
MAX_STRAFE = 5000.0
MAX_ACCEL = 15000.0
MAX_TURN = 1000.0
MAX_ROLL = 800.0

# Counter-turn and counter-roll coefficients
# These act like damping coefficients, but are only turned on when the axis's controls are released
CTURN_COEF = 700
CROLL_COEF = 700

class Avatar(gameobj.GameObj):
	"""The player character."""
	
	def __init__(self, pos):
		geom = ode.GeomCapsule(app.dyn_space, 0.25, 2.0) # 2.5 total length: a 2.0-long cylinder, and two 0.25-radius caps
		geom.coll_props = collision.Props()
		super(Avatar, self).__init__(pos = pos, body = sphere_body(80, 0.5), geom = geom)
		self._quad = gluNewQuadric()
		gluQuadricTexture(self._quad, GLU_TRUE)
		self._tex = resman.Texture("lava.png")
		self._relThrustVec = Point() # Indicates to the drawing routine how much the player is thrusting in each direction
		self._relTorqueVec = Point() # Indicates to the drawing routine how much the player is torquing along each axis
		self._relCTorqueVec = Point() # Indicates to the drawing routine how much automatic counter-torque is being applied along each axis
	
	def __del__(self):
		gluDeleteQuadric(self._quad)
	
	def step(self):	
		# TODO: Consider adding linear and angular velocity caps
		# TODO: Make joystick range circular (see example code on pygame help pages)
		tx, ty, tz = 0, 0, 0
		
		# X-strafing
		if app.axes[joy.LX] != 0.0:
			tx = -app.axes[joy.LX]*(MAX_STRAFE/app.maxfps)
			self.body.addRelForce((tx, 0, 0))
		
		# Y-strafing
		if app.axes[joy.LY] != 0.0:
			ty = -app.axes[joy.LY]*(MAX_STRAFE/app.maxfps)
			self.body.addRelForce((0, ty, 0))
		
		# Forward/backward
		if app.axes[joy.L2] != 0.0:
			tz = -app.axes[joy.L2]*(MAX_ACCEL/app.maxfps)
			self.body.addRelForce((0, 0, tz))
		elif app.axes[joy.R2] != 0.0:
			tz = app.axes[joy.R2]*(MAX_ACCEL/app.maxfps)
			self.body.addRelForce((0, 0, tz))
		
		self._relThrustVec = Point(tx, ty, tz)
		
		sx, sy, sz = 0, 0, 0
		csx, csy, csz = 0, 0, 0
		avel = self.body.vectorFromWorld(self.body.getAngularVel())

		# TODO : Maybe introduce counterturn assistance if user is turning opposite their angular velocity?
		
		# X-turn and X-counterturn
		if app.axes[joy.RX] != 0.0:
			sy = -app.axes[joy.RX]*(MAX_TURN/app.maxfps)
			self.body.addRelTorque((0.0, sy, 0.0))
		else:
			csy = avel[1]*-CTURN_COEF/app.maxfps
			self.body.addRelTorque((0.0, csy, 0.0))
		
		# Y-turn and Y-counterturn
		if app.axes[joy.RY] != 0.0:
			sx = app.axes[joy.RY]*(MAX_TURN/app.maxfps)
			self.body.addRelTorque((sx, 0.0, 0.0))
		else:
			csx = avel[0]*-CTURN_COEF/app.maxfps
			self.body.addRelTorque((csx, 0.0, 0.0))
		
		# Roll and counterroll
		if app.buttons[joy.L1] == joy.DOWN:
			sz = -MAX_ROLL/app.maxfps
			self.body.addRelTorque((0.0, 0.0, sz))
		elif app.buttons[joy.R1] == joy.DOWN:
			sz = MAX_ROLL/app.maxfps
			self.body.addRelTorque((0.0, 0.0, sz))
		else:
			csz = avel[2]*-CROLL_COEF/app.maxfps
			self.body.addRelTorque((0.0, 0.0, csz))

		self._reqlTorqueVec = Point(sx, sy, sz)
		self._reqlCTorqueVec = Point(csz, csy, csz)
	
	def indraw(self):
		# The cylinder body
		glTranslatef(0, 0, -0.75)
		glRotatef(180, 0, 0, 1)
		glEnable(GL_TEXTURE_2D)
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE)
		glBindTexture(GL_TEXTURE_2D, self._tex.glname)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
		gluCylinder(self._quad, 0.1, 0.1, 1.5, 30, 10)
		glDisable(GL_TEXTURE_2D)
		
		# The legs and feet
		glColor3f(*colors.red)
		glutSolidSphere(0.08, 15, 15)
		for sign in (-1, 1):
			glPushMatrix()
			glTranslatef(-sign*0.06, 0, 0)
			glRotatef(sign*10, 0, 1, 0)
			glTranslatef(0, 0, -0.8)
			gluCylinder(self._quad, 0.06, 0.06, 0.8, 10, 10)
			glutSolidSphere(0.08, 5, 5)
			glPopMatrix()
		
		# The jetpack
		glTranslatef(0, -0.03, 1)
		glColor3f(*colors.gray)
		glutSolidSphere(0.12, 15, 15)
		
		# Back up to the head
		glTranslatef(0, 0.05, 1)
		glColor3f(*colors.yellow)
		glutSolidSphere(0.15, 15, 15)
		
		# The arms and hands
		glPushMatrix()
		glTranslatef(0, 0, -0.7)
		glColor3f(*colors.red)
		glutSolidSphere(0.08, 15, 15)
		for sign in (-1, 1):
			glPushMatrix()
			glTranslatef(-sign*0.1, 0, 0)
			glRotatef(sign*10, 0, 1, 0)
			glTranslatef(0, 0, -0.6)
			gluCylinder(self._quad, 0.05, 0.05, 0.6, 10, 10)
			glutSolidSphere(0.08, 5, 5)
			glPopMatrix()
		glPopMatrix()
		
		# Back to the center for thrust indication cones
		glTranslatef(0, 0, -1)
		glColor3f(*colors.white)
		for offset, rot, value in (
			(0.50, (90, 0, 1, 0), self._relThrustVec[0]),
			(0.50, (-90, 1, 0, 0), self._relThrustVec[1]),
			(2.75, (180, 0, 1, 0), self._relThrustVec[2]),
		):
			if abs(value) > 0.01:
				glPushMatrix()
				# Orient so we're facing away from the direction we're accelerating in
				glRotatef(*rot)
				if value > 0:
					glTranslatef(0, 0, offset)
				else:
					glTranslatef(0, 0, -offset)
				glutSolidCone(0.15, value/100, 10, 5)
				glPopMatrix()
