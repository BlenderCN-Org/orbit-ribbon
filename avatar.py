from __future__ import division
import ode
import OpenGL
OpenGL.ERROR_CHECKING = False
OpenGL.ERROR_LOGGING = False
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

import app, gameobj, collision, joy, resman, anim
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

MODE_FLY, MODE_FLY_TO_PRERUN, MODE_PRERUN, MODE_PRERUN_TO_FLY = range(4)

class Avatar(gameobj.GameObj):
	"""The player character."""
	
	def __init__(self, oreman, pos, rot):
		self._oreman = oreman
		
		geom = ode.GeomCapsule(app.dyn_space, 0.25, 2.0) # 2.5 total length: a 2.0-long cylinder, and two 0.25-radius caps
		geom.coll_props = collision.Props()
		super(Avatar, self).__init__(pos = pos, rot = rot, body = sphere_body(80, 0.5), geom = geom)
		self._relThrustVec = Point() # Indicates to the drawing routine how much the player is thrusting in each direction
		self._relTorqueVec = Point() # Indicates to the drawing routine how much the player is torquing along each axis
		self._relCTorqueVec = Point() # Indicates to the drawing routine how much automatic counter-torque is being applied along each axis
		
		self._anim = anim.AnimManager(ore_anim = self._oreman.animations["LIBAvatar-FlyToPrerun"], lock = True)
		self._mode = MODE_FLY
	
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
		
		# Moving between fly mode and prerun mode
		if (
			(app.buttons[joy.L1] == joy.DOWN and app.buttons[joy.R1] == joy.DOWN)
			or ((self._mode == MODE_FLY_TO_PRERUN or self._mode == MODE_PRERUN) and (app.buttons[joy.L1] == joy.DOWN or app.buttons[joy.R1] == joy.DOWN))
		):
			if self._mode == MODE_FLY:
				self._anim = anim.AnimManager(ore_anim = self._oreman.animations["LIBAvatar-FlyToPrerun"])
				self._mode = MODE_FLY_TO_PRERUN
			elif self._mode == MODE_PRERUN_TO_FLY:
				self._anim.reverse = False
				self._mode = MODE_FLY_TO_PRERUN
			elif self._mode == MODE_FLY_TO_PRERUN and self._anim.on_last_frame():
				self._anim = anim.AnimManager(ore_anim = self._oreman.animations["LIBAvatar-FlyToPrerun"], lock = True, reverse = True)
				self._mode = MODE_PRERUN
		else:
			if self._mode == MODE_PRERUN:
				self._anim = anim.AnimManager(ore_anim = self._oreman.animations["LIBAvatar-FlyToPrerun"], reverse = True)
				self._mode = MODE_PRERUN_TO_FLY
			elif self._mode == MODE_FLY_TO_PRERUN:
				self._anim.reverse = True
				self._mode = MODE_PRERUN_TO_FLY
			elif self._mode == MODE_PRERUN_TO_FLY and self._anim.on_last_frame():
				self._anim = anim.AnimManager(ore_anim = self._oreman.animations["LIBAvatar-FlyToPrerun"], lock = True)
				self._mode = MODE_FLY
		
		# Roll
		if app.buttons[joy.L1] == joy.DOWN and app.buttons[joy.R1] == joy.UP:
			self.running = False
			sz = -MAX_ROLL/app.maxfps
			self.body.addRelTorque((0.0, 0.0, sz))
		elif app.buttons[joy.R1] == joy.DOWN and app.buttons[joy.L1] == joy.UP:
			self.running = False
			sz = MAX_ROLL/app.maxfps
			self.body.addRelTorque((0.0, 0.0, sz))
		
		# Counter-roll
		if app.buttons[joy.L1] == app.buttons[joy.R1]: # If both L1 and R1 are up or both are down
			csz = avel[2]*-CROLL_COEF/app.maxfps
			self.body.addRelTorque((0.0, 0.0, csz))
		
		self._reqlTorqueVec = Point(sx, sy, sz)
		self._reqlCTorqueVec = Point(csz, csy, csz)
		
		self._anim.cur_frame()
	
	def indraw(self):
		# The body model
		self._anim.cur_frame_static().draw_gl()
		
		# Thrust indication cones
		glMaterialfv(GL_FRONT, GL_DIFFUSE, (0.8, 0.8, 0.8, 1.0))
		glMaterialfv(GL_FRONT, GL_SPECULAR, (1.0, 1.0, 1.0, 1.0))
		for offset, rot, value in (
			(0.50, (-90, 0, 1, 0), self._relThrustVec[0]),
			(0.50, (90, 1, 0, 0), self._relThrustVec[1]),
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
