from __future__ import division
import ode

import app, gameobj, collision, resman, anim, inputs
from geometry import *
from util import *
from gl import *

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
		# TODO: Set a maximum total acceleration, then force accel vector to be no longer than that magnitude
		tx, ty, tz = 0, 0, 0
		
		# X-strafing
		v = app.input_man.intent_channels[inputs.INTENT_TRANS_X].value()
		if v != 0.0:
			tx = -v*(MAX_STRAFE/app.maxfps)
			self.body.addRelForce((tx, 0, 0))
		
		# Y-strafing
		v = app.input_man.intent_channels[inputs.INTENT_TRANS_Y].value()
		if v != 0.0:
			ty = -v*(MAX_STRAFE/app.maxfps)
			self.body.addRelForce((0, ty, 0))
		
		# Forward/backward
		v = app.input_man.intent_channels[inputs.INTENT_TRANS_Z].value()
		if v != 0.0:
			tz = v*(MAX_ACCEL/app.maxfps)
			self.body.addRelForce((0, 0, tz))
		
		self._relThrustVec = Point(tx, ty, tz)
		
		sx, sy, sz = 0, 0, 0
		csx, csy, csz = 0, 0, 0
		avel = self.body.vectorFromWorld(self.body.getAngularVel())
		
		# TODO : Maybe introduce counterturn assistance if user is turning opposite their angular velocity?
		
		# X-turn and X-counterturn
		v = app.input_man.intent_channels[inputs.INTENT_ROTATE_Y].value()
		if v != 0.0:
			sy = -v*(MAX_TURN/app.maxfps)
			self.body.addRelTorque((0.0, sy, 0.0))
		else:
			csy = avel[1]*-CTURN_COEF/app.maxfps
			self.body.addRelTorque((0.0, csy, 0.0))
		
		# Y-turn and Y-counterturn
		v = app.input_man.intent_channels[inputs.INTENT_ROTATE_X].value()
		if v != 0.0:
			sx = v*(MAX_TURN/app.maxfps)
			self.body.addRelTorque((sx, 0.0, 0.0))
		else:
			csx = avel[0]*-CTURN_COEF/app.maxfps
			self.body.addRelTorque((csx, 0.0, 0.0))
		
		# Moving between fly mode and prerun mode
		# Do not leave prerun mode once entered until INTENT_RUN_STANCE is entirely off
		if (
			app.input_man.intent_channels[inputs.INTENT_RUN_STANCE].is_on() or
			(
				(self._mode == MODE_FLY_TO_PRERUN or self._mode == MODE_PRERUN) and
				app.input_man.intent_channels[inputs.INTENT_RUN_STANCE].is_partially_on()
			)
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
		v = app.input_man.intent_channels[inputs.INTENT_ROTATE_Z].value()
		if v != 0.0:
			sz = v*(MAX_ROLL/app.maxfps)
			self.body.addRelTorque((0.0, 0.0, sz))
		else:
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
