from __future__ import division

import app
from geometry import *

class Camera:
	"""A superclass for classes which are responsible for controlling the 3D camera.
	
	Subclasses need only to implement the get_camvals() method which should return values
	that can be passed to gluLookAt.
	"""
	
	def get_camvals(self):
		"""Returns a 9-tuple (triplets of camera position, camera target, and camera up vector), suitable for gluLookAt."""
		raise NotImplementedError
	
	def get_position(self):
		"""Returns a Point with the camera's position, based on the return value of get_camvals()."""
		return Point(*(self.get_camvals()[0:3]))


class FixedCamera(Camera):
	"""A Camera that simply sits in a given position.
	
	Data attributes:
	position - A Point indicating where the camera is.
	target - A Point indicating what the camera is to look at.
	up_vec - A Point containing a vector for the camera's up direction.
	"""
	
	def __init__(self, position, target, up_vec):
		self.position = position
		self.target = target
		self.up_vec = up_vec
	
	def get_camvals(self):
		p, t, u = self.position, self.target, self.up_vec
		return (p[0], p[1], p[2], t[0], t[1], t[2], u[0], u[1], u[2])


class FollowCamera(Camera):
	"""A Camera that follows a given GameObj.
	
	Data attributes:
	target_obj - The GameObj to be tracked, which must have an ODE body.
	cam_offset - A Point containing a vector indicating the camera position's offset from target_obj in target object's coord system.
	target_offset - A Point containing a vector indicating the camera's look target's offset from target_obj in target object's coord system.
	up_vec - A Point containing a vector indicating the camera's up direction in target object's local coord system.
	"""
	
	def __init__(self, target_obj, cam_offset = None, target_offset = None, up_vec = None):
		"""Creates a FollowCamera. If offset not supplied, a reasonable one is used by default."""
		self.target_obj = target_obj
		
		if cam_offset is None:
			cam_offset = Point(0, 1.1, -6)
		self.cam_offset = cam_offset
		
		if target_offset is None:
			target_offset = Point(0, 1.1, 0)
		self.target_offset = target_offset

		if up_vec is None:
			up_vec = Point(0, 1, 0)
		self.up_vec = up_vec
	
	def get_camvals(self):
		b = self.target_obj.body
		p = Point(*b.getRelPointPos(self.cam_offset))
		t = Point(*b.getRelPointPos(self.target_offset))
		u = Point(*b.vectorToWorld(self.up_vec))
		return (p[0], p[1], p[2], t[0], t[1], t[2], u[0], u[1], u[2])
