from __future__ import division

class Camera:
	"""A superclass for classes which are responsible for controlling the 3D camera.
	
	Subclasses need only to implement the get_camvals() method which should return values
	that can be passed to gluLookAt.
	"""
	
	def get_camvals(self):
		"""Returns a 9-tuple (triplets of camera position, camera target, and camera up vector), suitable for gluLookAt."""
		raise NotImplementedError

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
	offset - A Point containing a vector indicating the offset from target_obj.
		The y-part of this vector is used to offset the camera target, so that we look
		past the target object rather than straight at it.
	"""
	
	def __init__(self, target_obj, offset = None):
		"""Creates a FollowCamera. If offset not supplied, a reasonable one is used by default."""
		self.target_obj = target_obj
		
		if offset is None:
			offset = Point(0, 1.1, -6)
		self.offset = offset
	
	def get_camvals(self):
		b = self.target_obj.body
		app.camera_tgt = Point(*b.getRelPointPos((0, self.offset[1], 0)))
		app.camera = Point(*b.getRelPointPos(*self.offset))
		app.camera_up = Point(*b.vectorToWorld((self.offset[0], self.offset[1]+1, self.offset[2])))
