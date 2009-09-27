from __future__ import division

import math, ode

import app
from geometry import *
from util import *
from gl import *

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
	body -- The ODE body used for physical dynamics. Defaults to None.
		This can be None if you don't want an object to ever move.
		Do not change this value once the GameObj is created.
		Also, the body will be given a "gameobj" attribute so you can
		get back to a GameObj from its body.
	geom -- The ODE geometry used for collision detection.
		This can be None if you don't want an object to ever collide.
		Do not change this value once the GameObj is created.
		You may also supply a Space if you want to have several associated geoms.
		This will be automatically associated with body if body is also supplied.
		Also, the geom and all its children will be given a "gameobj" attribute
		so you can get back to a GameObj from its geom.
		GameObj will descend recursively into Spaces to find
		all non-Space Geoms to associate with body and assign "gameobj" attrs.
	velDamp - A 3-tuple of drag coefficients for the body's linear velocity axes
	angDamp - A 3-tuple of drag coefficients for the body's angular velocity axes
		These are not real drag coefficients like an aerodynamics engineers
		would use, just convenience values which are multiplied by the object's
		velocity to find how much drag force per second is applied.
	"""
	
	# TODO: Add a del routine that removs bodies and geoms
	
	def __init__(self, pos = None, rot = None, body = None, geom = None):
		"""Creates a GameObj. Pos and rot given overrides the position of body and/or geom."""
		self.body = body
		self.geom = geom
		if self.body is not None:
			self.body.gameobj = self
		# If we got both a body and a geom, associate them, if necessary descending recursively into spaces
		if self.body is not None and self.geom is not None:
			self._assoc_geom(self.geom)
		
		#Overwrite ODE position with the passed-in position, or origin by default
		if pos == None:
			pos = Point()
		self.pos = pos # Calls _set_pos
		
		#Also overwrite ODE rotation matrix
		if rot == None:
			rot = (1, 0, 0,   0, 1, 0,   0, 0, 1,) # Column-major 3x3 matrix
		self.rot = rot # Calls _set_rot
		
		#Default damping values (can be adjusted by subclass/external user as needed)
		self.velDamp = Point(DEFAULT_VEL_DAMP_COEF, DEFAULT_VEL_DAMP_COEF, DEFAULT_VEL_DAMP_COEF)
		self.angDamp = Point(DEFAULT_ANG_DAMP_COEF, DEFAULT_ANG_DAMP_COEF, DEFAULT_ANG_DAMP_COEF)
	
	def __str__(self):
		return "(%s)" % (
			str(self.pos),
		)
	
	def _assoc_geom(self, tgt):
		tgt.gameobj = self
		if tgt.isSpace():
			for subtgt in tgt:
				self._assoc_geom(subtgt)
		else:
			tgt.setBody(self.body)
	
	def _get_pos(self): return self._pos
	
	def _set_pos(self, pos):	
		self._pos = pos
		
		#If body and geom are connected, setting pos or rot in one sets it in both
		if self.body != None: self._set_ode_pos(self.body)
		elif self.geom != None: self._set_ode_pos(self.geom)
	
	def _get_vel(self):
		if self.body != None:
			return Point(*self.body.getLinearVel())
		else:
			return Point()
	
	def _set_vel(self, vel):
		if self.body != None:
			self.body.setLinearVel(vel)
	
	def _get_rot(self): return self._rot
	
	def _set_rot(self, rot):
		if len(rot) != 9:
			raise TypeError("Supplied rot value must be a 9-tuple")
		self._rot = tuple(rot)
		
		#If body and geom are connected, setting pos or rot in one sets it in both
		if self.body != None: self._set_ode_rot(self.body)
		elif self.geom != None: self._set_ode_rot(self.geom)
	
	def _fetch_ode_from(self, odething):
		"""Loads position and rotation data from the given ODE object (either a body or a geom)."""
		
		#Get the position
		odepos = odething.getPosition()
		self._pos = Point(odepos[0], odepos[1], odepos[2])
		
		# Get the rotation matrix
		oderot = odething.getRotation()
		self._rot = ( # Convert row-major to column-major
			oderot[0], oderot[3], oderot[6],
			oderot[1], oderot[4], oderot[7],
			oderot[2], oderot[5], oderot[8],
		)
	
	def _set_ode_rot(self, odething):
		"""Sets the rotation in an ODE object (body or geom) from the GameObj's rotation matrix."""
		if isinstance(odething, ode.SpaceBase):
			for subthing in odething:
				self._set_ode_rot(subthing)
		else:
			r = self._rot
			odething.setRotation(( # Column-major to row-major
				r[0], r[3], r[6],
				r[1], r[4], r[7],
				r[2], r[5], r[8],
			))
	
	def _set_ode_pos(self, odething):
		"""Sets the position in an ODE object (body or geom) from the GameObj's position."""
		if isinstance(odething, ode.SpaceBase):
			for subthing in odething:
				self._set_ode_pos(subthing)
		else:
			odething.setPosition(self._pos)
	
	def sync_ode(self):
		"""Sets position and rotation based on the ODE state.
		
		This is called automatically by the main loop after the simstep is ran.
		"""
		
		if self.body != None:
			self._fetch_ode_from(self.body)
		elif self.geom != None and not self.geom.isSpace():
			self._fetch_ode_from(self.geom)
	
	def step(self):
		"""Does whatever required for each simulation step of the object; can be implemented by subclasses."""
		pass
	
	def indraw(self):
		"""Does whatever is required to draw the object; can be implemented by subclasses.
		
		When this is called, the correct GL matrix is already in place, and matrix changes made within will be popped afterwards."""
		pass
	
	def draw(self):
		"""If in range, draws the object; pushes correct GL matrix, calls indraw(), restores GL.
		
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
	
	def indistdraw(self):
		"""Does whatever is required to draw the object when it is farther than GAMEPLAY_CLIP_DIST away.
		
		When this is called, GL matrix has not been changed from regular state.
		"""
		pass
	
	def distdraw(self):
		"""If outside of drawable range, draws billboard of object; sets GL state, calls indistdraw, restores GL.

		Subclasses should override indistdraw, not this.
		"""
		self.indistdraw()
	
	def freeze(self):
		"""Kills the object's linear and angular velocity.
		
		This can do weird things to the simulation, so only use it for debugging.
		"""
		if self.body == None:
			return
		self.body.setLinearVel((0, 0, 0))
		self.body.setAngularVel((0, 0, 0))
	
	def damp(self):
		"""Applies damping to linear and angular velocity. Called by app module after each object's step."""
		if self.body == None:
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
