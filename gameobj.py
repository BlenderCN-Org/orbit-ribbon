import math, ode
from OpenGL.GL import *

import app
from geometry import *
from util import *

DEFAULT_VEL_DAMP_COEF = 0.15
DEFAULT_ANG_DAMP_COEF = 0.15

class GameObj(object):
	"""The base class for in-game objects of all kinds.
	
	Data attributes:
	pos -- A Point of the absolute location of the center of the object.
	rot -- A 3x3 column-major rotation matrix describing the current orientation of the object.
	vel -- The linear velocity of the object.
		Set these instead of calling methods on body or geom: ODE
		will automatically be updated when these are set, and
		changes in ODE will be restored back to these when restorePhys()
		is called.
	body -- The ODE body used for physical dynamics.
		This can be None if you don't want an object to ever move.
		Setting body automatically adjusts geom as needed
		(i.e. setting collision groups, calling set_body(), etc).
		The old body is also automatically unassociated and destroyed.
		Additionally, pos and rot are automatically loaded from body
		after it is set, and geom's pos and rot are overwritten.
		Also, the body will be given a "gameobj" attribute so you can
		get back to a GameObj from its body.
	geom -- The ODE geometry used for collision detection.
		This can be None if you don't want an object to ever collide.
		Setting geom will cause it to be associated with
		body, if there is one set. The old geom is also automatically
		unassociated and destroyed. Additionally, pos and rot
		are automatically loaded from geom after it is set, and body's
		pos and rot are overwritten. Also, the geom will be given
		a "gameobj" attribute so you can get back to a GameObj from
		its geom.
	velDamp - A 3-tuple of drag coefficients for the body's linear velocity axes
	angDamp - A 3-tuple of drag coefficients for the body's angular velocity axes
		These are not real drag coefficients like an aerodynamics engineers would use, just
		convenience values which are multiplied by the square of the object's
		velocity to find how much force per second is applied.
	"""
	
	def __init__(self, pos = None, rot = None, body = None, geom = None):
		"""Creates a GameObj. Pos and rot given overrides the position of body and/or geom."""
		self._body = None
		self._geom = None
		self.body = body #This calls the smart setter,
		self.geom = geom #This also calls smart setter, which associates if possible
		
		#Overwrite ODE position with the passed-in position, or origin by default
		if pos == None:
			pos = Point()
		self.pos = pos
		
		#Also overwrite ODE rotation matrix
		if rot == None:
			rot = (1, 0, 0,   0, 1, 0,   0, 0, 1,) # Column-major 3x3 matrix
		self.rot = rot
		
		#Default damping values (can be adjusted by subclass/external user as needed)
		self.velDamp = Point(DEFAULT_VEL_DAMP_COEF, DEFAULT_VEL_DAMP_COEF, DEFAULT_VEL_DAMP_COEF)
		self.angDamp = Point(DEFAULT_ANG_DAMP_COEF, DEFAULT_ANG_DAMP_COEF, DEFAULT_ANG_DAMP_COEF)
	
	def __str__(self):
		return "(%s)" % (
			str(self.pos),
		)
	
	def _get_geom(self): return self._geom
	
	def _set_geom(self, geom):
		#Remove and disassociate existing geom if any
		if self._geom != None:
			self._geom.setBody(None)
			app.odeworld.remove(self._geom)
		
		#Set the new geom, load its rot and pos, and associate it if possible
		self._geom = geom
		if self._geom != None:
			self._geom.gameobj = self
			self._fetch_ode_from(self._geom)
			if self._body != None:
				self._set_ode_pos(self._body)
				self._set_ode_rot(self._body)
				self._geom.setBody(self._body)
	
	def _get_body(self): return self._body
	
	def _set_body(self, body):
		#Remove and disassociate existing body if any
		if self._geom != None:
			self._geom.setBody(None)
		
		#FIXME: Figure out a way to actually delete bodies from the world
		
		#Set the new body, load its position and rotation data, and associate it if possible
		self._body = body
		if self._body != None:
			self._body.gameobl = self
			self._fetch_ode_from(self._body)
			if self._geom != None:
				self._set_ode_pos(self._geom)
				self._set_ode_rot(self._geom)
		if self._geom != None:
			self._geom.setBody(self._body)
	
	def _get_pos(self): return self._pos
	
	def _set_pos(self, pos):	
		self._pos = pos
		
		#If body and geom are connected, setting pos or rot in one sets it in both
		if self._body != None: self._set_ode_pos(self._body)
		elif self._geom != None: self._set_ode_pos(self._geom)
	
	def _get_vel(self):
		if self._body != None:
			return Point(*self.body.getLinearVel())
		else:
			return Point()

	def _set_vel(self, vel):
		if self._body != None:
			self.body.setLinearVel(vel)
	
	def _get_rot(self): return self._rot
	
	def _set_rot(self, rot):
		if len(rot) != 9:
			raise TypeError("Supplied rot value must be a 9-tuple")
		self._rot = tuple(rot)
		
		#If body and geom are connected, setting pos or rot in one sets it in both
		if self._body != None: self._set_ode_rot(self._body)
		elif self._geom != None: self._set_ode_rot(self._geom)
	
	def _fetch_ode_from(self, odething):
		"""Loads position and rotation data from the given ODE object (either a body or a geom)."""
		
		#Get the position
		odepos = odething.getPosition()
		self._pos = Point(odepos[0], odepos[1], odepos[2])
		
		# Get the rotation matrix
		oderot = odething.getRotation()
		self._rot = ( # Convert 3x3 row-major to 4x4 column-major
			oderot[0], oderot[3], oderot[6],
			oderot[1], oderot[4], oderot[7],
			oderot[2], oderot[5], oderot[8],
		)
	
	def _set_ode_rot(self, odething):
		"""Sets the rotation in an ODE object (body or geom) from the GameObj's rotation matrix."""
		
		r = self._rot
		odething.setRotation(( # Column-major to row-major
			r[0], r[3], r[6],
			r[1], r[4], r[7],
			r[2], r[5], r[8],
		))
	
	def _set_ode_pos(self, odething):
		"""Sets the position in an ODE object (body or geom) from the GameObj's position."""
		odething.setPosition(self._pos)
	
	def sync_ode(self):
		"""Sets position and rotation based on the ODE state.
		
		This is called automatically by the main loop after the simstep is ran.
		"""
		
		if self._body != None:
			self._fetch_ode_from(self._body)
		elif self._geom != None:
			self._fetch_ode_from(self._geom)
	
	def step(self):
		"""Does whatever required for each simulation step of the object; can be implemented by subclasses."""
		pass
	
	def indraw(self):
		"""Does whatever is required to draw the object; can be implemented by subclasses.
		
		When this is called, the correct GL matrix is already in place, and matrix changes made within will be popped afterwards."""
		pass
	
	def draw(self):
		"""Draws the object; pushes correct GL matrix, calls indraw(), restores GL.
		
		Subclasses should override indraw(), not this.
		"""
		glPushMatrix()
		glTranslatef(*self.pos)
		r = self.rot
		glMultMatrixf(( # Pad out with 4th row and column for OpenGL
			r[0], r[1], r[2], 0,
			r[3], r[4], r[5], 0,
			r[6], r[7], r[8], 0,
			   0,    0,    0, 1,
		))
		
		self.indraw()
		
		glPopMatrix()
	
	def freeze(self):
		"""Kills the object's linear and angular velocity."""
		if self._body == None:
			return
		self._body.setLinearVel((0, 0, 0))
		self._body.setAngularVel((0, 0, 0))
	
	def damp(self):
		"""Applies damping to linear and angular velocity. Called by app module after each object's step."""
		if self._body == None:
			return
		
		vel = self.body.vectorFromWorld(self.body.getLinearVel())
		self.body.addRelForce(tuple(
			[vel[n] * -self.velDamp[n]/app.maxfps for n in range(3)]
		))
		avel = self.body.vectorFromWorld(self.body.getAngularVel())
		self.body.addRelTorque(tuple(
			[avel[n] * -self.angDamp[n]/app.maxfps for n in range(3)]
		))
	
	pos = property(_get_pos, _set_pos)
	vel = property(_get_vel, _set_vel)
	rot = property(_get_rot, _set_rot)
	body = property(_get_body, _set_body)
	geom = property(_get_geom, _set_geom)
