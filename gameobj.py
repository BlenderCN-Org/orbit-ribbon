import math, ode
from OpenGL.GL import *

import app, util
from geometry import *

class GameObj(object):
	# FIXME - Bring back ang as something useful in 3 dimensions
	"""The base class for in-game objects of all kinds.
	
	Data attributes:
	pos -- A Point of the absolute location of the center of the object.
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
		Additionally, pos and ang are automatically loaded from body
		after it is set, and geom's ang and pos are overwritten.
		Also, the body will be given a "gameobj" attribute so you can
		get back to a GameObj from its body.
	geom -- The ODE geometry used for collision detection.
		This can be None if you don't want an object to ever collide.
		Setting geom will cause it to be associated with
		body, if there is one set. The old geom is also automatically
		unassociated and destroyed. Additionally, pos and ang
		are automatically loaded from geom after it is set, and body's
		ang and pos are overwritten. Also, the geom will be given
		a "gameobj" attribute so you can get back to a GameObj from
		its geom.
	"""
	
	def __init__(self, pos = None, body = None, geom = None):
		"""Creates a GameObj. Pos and given overrides the position of body and/or geom."""
		self._body = None
		self._geom = None
		self.body = body #This calls the smart setter,
		self.geom = geom #This also calls smart setter, which associates if possible
		
		if pos == None: self.pos = Point()
		else: self.pos = pos #Overwrite ODE position with the passed-in position
		#self.ang = ang #Overwrite ODE angle too
		
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
		
		#Set the new geom, load its ang and pos, and associate it if possible
		self._geom = geom
		if self._geom != None:
			self._geom.gameobj = self
			self._fetch_ode_from(self._geom)
			if self._body != None:
				self._set_ode_pos(self._body)
				#self._set_ode_ang(self._body)
				self._geom.setBody(self._body)
	
	def _get_body(self): return self._body
	
	def _set_body(self, body):
		#Remove and disassociate existing body if any
		if self._geom != None:
			self._geom.setBody(None)
		
		#FIXME: Figure out a way to actually delete bodies from the world
		
		#Set the new body, load its positional data, and associate it if possible
		self._body = body
		if self._body != None:
			self._body.gameobl = self
			self._fetch_ode_from(self._body)
			if self._geom != None:
				self._set_ode_pos(self._geom)
				#self._set_ode_ang(self._geom)
		if self._geom != None:
			self._geom.setBody(self._body)
	
	def _get_pos(self): return self._pos
	
	def _set_pos(self, pos):	
		self._pos = pos
		
		#If body and geom are connected, setting pos or ang in one sets it in both
		if self._body != None: self._set_ode_pos(self._body)
		elif self._geom != None: self._set_ode_pos(self._geom)
	
	def _get_vel(self):
		if self._body != None:
			return Point(*self.body.getLinearVel()[0:3])
		else:
			return Point()

	def _set_vel(self, vel):
		if self._body != None:
			self.body.setLinearVel(vel)
	
#	def _get_ang(self): return self._ang
#	
#	def _set_ang(self, ang):
#		#Wrap to [0-1) revolutions
#		self._ang = ang % 1
#		
#		#If body and geom are connected, setting pos or ang in one sets it in both
#		if self._body != None: self._set_ode_ang(self._body)
#		elif self._geom != None: self._set_ode_ang(self._geom)
	
	
	def _fetch_ode_from(self, odething):
		"""Loads positional data from the given ODE object (either a body or a geom)."""
		
		#Get the position
		odepos = odething.getPosition()
		self._pos = Point(odepos[0], odepos[1], odepos[2])
		
		#Convert ccw radians to cw revolutions
		#rot = odething.getRotation()
		#uncos = math.acos(rot[0])
		#unsin = math.asin(rot[1])
		#self._ang = uncos/(-2.0 * math.pi)
		#if unsin < 0:
		#	self._ang = -self._ang

		#Wrap to [0-1) revolutions
		#self._ang = self.ang % 1
	
#	def _set_ode_ang(self, odething):
#		"""Sets the angle in an ODE object (body or geom) from the GameObj's angle.
#		
#		Converts from GameObj angles (cw revolutions) to ODE angles (ccw radians).
#		"""
#		a = util.rev2rad(self._ang)
#		s = math.sin(a)
#		c = math.cos(a)
#		rotmatr = (c, s, 0.0, -s, c, 0.0, 0.0, 0.0, 1.0)
#		odething.setRotation(rotmatr)
	
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
		
		When this is called, the correct GL matrix is already in place, and matrix changes made within will be popped."""
		pass
		
	def draw(self):
		"""Draws the object; pushes correct GL matrix, calls indraw(), restores GL."""
		glPushMatrix()
		glTranslatef(self.pos[0], self.pos[1], self.pos[2])
		# FIXME: Must rotate with angle here
		#if self.ang > 0.00001:
		#	glRotatef(util.rev2deg(self.ang), 0, 0, 1)
		
		self.indraw()
		
		glPopMatrix()
	
	def freeze(self):
		"""Kills the object's linear and angular velocity."""
		if self._body == None:
			return
		
		self._body.setLinearVel((0, 0, 0))
		self._body.setAngularVel((0, 0, 0))
	
	pos = property(_get_pos, _set_pos)
	vel = property(_get_vel, _set_vel)
	#ang = property(_get_ang, _set_ang)
	body = property(_get_body, _set_body)
	geom = property(_get_geom, _set_geom)
