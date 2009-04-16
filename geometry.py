from __future__ import division
import math, copy
import util

def _coordlike_copy(obj):
	"""Given an object, if it's not already a derivative of _CoordLike,
	then it returns a new _CoordLike with that object in both fields. Otherwise,
	it makes a copy of the object and returns it."""
	if isinstance(obj, _CoordLike):
		return copy.copy(obj)
	else:
		return _CoordLike(obj)

def _coordlikeify(obj):
	"""Like _coordlike_copy, but if the object is already a derivative of _CoordLike,
	just returns the object itself (not a copy)."""
	if isinstance(obj, _CoordLike):
		return obj
	else:
		return _CoordLike(obj)

class _CoordLike(list):
	"""Used to implement the magic of Point.
	
	Basically, just a three element list that supports arithmetic, both
	between two _CoordLikes and between a _CoordLike and a number,
	which is just treated like a _CoordLike with all fields set
	to that number."""
	
	def __init__(self, a, b = None, c = None):
		"""
		Creates a _CoordLike.
		
		If only one argument is specified, it's copied into all fields."""
		
		self.append(a)
		if b == None:
			self.append(a)
			self.append(a)
		else:
			self.append(b)
			self.append(c)
	
	def __str__(self):
		return ",\n".join(["%+ 13.4f" % x for x in self])
	
	def __repr__(self):
		return ",\n".join(["%+ 13.4f" % x for x in self])
	
	def __add__(self, y):
		ret = _coordlike_copy(self)
		ret += y
		return ret
	
	def __radd__(self, y):
		return self.__add__(y)
	
	def __iadd__(self, y):
		cy = _coordlikeify(y)
		self[0] += cy[0]
		self[1] += cy[1]
		self[2] += cy[2]
		return self
	
	def __sub__(self, y):
		return self + (-y)
	
	def __rsub__(self, y):
		return (-self) + y
	
	def __isub__(self, y):
		self += (-y)
		return self
	
	def __mul__(self, y):
		ret = _coordlike_copy(self)
		ret *= y
		return ret
	
	def __rmul__(self, y):
		return self.__mul__(y)
	
	def __imul__(self, y):
		cy = _coordlike_copy(y)
		self[0] *= cy[0]
		self[1] *= cy[1]
		self[2] *= cy[2]
		return self
	
	def __div__(self, y):
		ret = _coordlike_copy(self)
		ret /= y
		return ret
	
	def __rdiv__(self, y):
		return _coordlikeify(y).__div__(self)
	
	def __idiv__(self, y):
		cy = _coordlikeify(y)
		self[0] /= cy[0]
		self[1] /= cy[1]
		self[2] /= cy[2]
		return self
	
	def __truediv__(self, y):
		return self.__div__(y)
	
	def __rtruediv__(self, y):
		return _coordlikeify(y).__truediv__(self)
	
	def __itruediv__(self, y):
		return self.__idiv__(y)
	
	def __floordiv__(self, y):
		ret = _coordlike_copy(self)
		ret //= y
		return ret
	
	def __rfloordiv__(self, y):
		return _coordlikeify(y).__floordiv__(self)
	
	def __ifloordiv__(self, y):
		cy = _coordlikeify(y)
		self[0] //= cy[0]
		self[1] //= cy[1]
		self[2] //= cy[2]
		return self
	
	def __neg__(self):
		c = copy.copy(self)
		c[0] = -c[0]
		c[1] = -c[1]
		c[2] = -c[2]
		return c
	
	def __abs__(self):
		c = copy.copy(self)
		c[0] = abs(c[0])
		c[1] = abs(c[1])
		c[2] = abs(c[2])
		return c
	
	def near_to(self, y):
		"""Returns true if this point and the other are close enough to be considered equal."""
		delta = 0.0001
		return (abs(self[0]-y[0])<delta and abs(self[1]-y[1])<delta and abs(self[2]-y[2])<delta)


class Point(_CoordLike, object):
	"""Represents a point in 3D space.
	
	This class could also be used to represent vectors or offsets
	
	Data attributes:
	x, y, z -- The coordinates (mean the same thing as pt[0], pt[1], and pt[2] respectively).
	"""
			
	def __init__(self, x = 0, y = 0, z = 0):
		super(Point, self).__init__(x, y, z)
	
	def __copy__(self):
		return Point(self[0], self[1], self[2])
	
	def __deepcopy__(self, memo):
		return self.__copy__()
	
	def mag(self):
		"""Returns the distance between the origin and this point."""
		return math.sqrt(math.sqrt(self[0]**2.0 + self[1]**2.0)**2.0 + self[2]**2.0)
	
	def dist_to(self, other):
		"""Returns the distance between this point and another."""
		return math.sqrt(math.sqrt((self[0]-other[0])**2.0 + (self[1]-other[1])**2.0)**2.0 + (self[2]-other[2])**2.0)
	
	def ang(self):
		# FIXME : Needs to be 3D
		"""Returns the angle from the origin to this point."""
		return Point().ang_to(self)
	
	def ang_to(self, other):
		# FIXME : Needs to be 3D
		"""Returns the angle from this point to another in clockwise revolutions.
		
		If other is directly to the right of this point, then the angle is zero."""
		return math.atan2(other[1]-self[1], other[0]-self[0])/(2*math.pi)
	
	def to_length(self, len = 1.0):
		"""Returns a coord of a given length, but in the same direction from the origin."""
		if len == 0.0:
			return Point()
		oldlen = self.mag()
		ret = Point(self[0], self[1], self[2])
		if oldlen != 0.0:
			ret[0] *= len/oldlen
			ret[1] *= len/oldlen
			ret[2] *= len/oldlen
		return ret
	
	def rot(self, cen, ang):
		# FIXME : Needs to be 3D
		"""Returns this point rotated around a center a given number of cw revolutions."""
	
		if abs(ang) < 0.000001:
			return copy.copy(self)
		a = util.rev2rad(ang) #Convert to ccw radians
		h = cen.dist_to(self) #Radius of circle
		b = util.rev2rad(cen.ang_to(self))
		return Point(h*math.cos(a+b)+cen[0], -1*h*math.sin(a+b)+cen[1])
	
	def _get_x(self): return self[0]
	def _set_x(self, x): self[0] = x
	def _get_y(self): return self[1]
	def _set_y(self, y): self[1] = y
	def _get_z(self): return self[2]
	def _set_z(self, z): self[2] = z
	x = property(_get_x, _set_x)
	y = property(_get_y, _set_y)
	z = property(_get_z, _set_z)
	

class Line:
	"""Represents a line segment from one point to another.
	
	Units are in game meters.
	
	Data attributes:
	a, b -- Two Points defining the ends of the line.
	"""
	
	def __init__(self, a, b):
		self.a = a
		self.b = b
	
	def __copy__(self):
		return Line(self.a, self.b)
	
	def __deepcopy__(self, memo):
		return Line(self.a.__copy__(), self.b.__copy__())
		
	def nearest_pt_to(self, p):
		"""Returns the nearest point on the line segment to an arbitrary point."""
		# FIXME : Need to make 3D
		u = (p.x-self.a.x)*(self.b.x-self.a.x) + (p.y-self.a.y)*(self.b.y-self.a.y)
		u /= abs((self.a.x-self.b.x)**2 + (self.a.y-self.b.y)**2)
		u = min(max(0.0, u), 1.0)
		return Point(self.a.x + u*(self.b.x-self.a.x), self.a.y + u*(self.b.y-self.a.y))
