from __future__ import division

import pygame, os, ode, math

import app, collision
from geometry import *

INTERP_MODE_LINEAR, INTERP_MODE_SMOOTHED, INTERP_MODE_LOG_DOWN = range(3)
def interpolate(a, b, x, mode):
	"""Given two acceptable values of the same type, an x value in [0.0, 1.0], and an INTERP_MODE_* value, returns an interpolated value.
	
	Acceptable values include: numbers, tuples of numbers, and SkyStuff instances.
	
	If x is 0.0 or less, returns a. If x is 1.0 or greater, returns b. Otherwise, returns something in between.
	"""
	import sky
	return_interpreter = lambda x: x
	if isinstance(a, float) or isinstance(a, int):
		if not (isinstance(b, float) or isinstance(b, int)):
			raise RuntimeError("Tried to interpolate number with non-number!")
		a = (a,)
		b = (b,)
		return_interpreter = lambda x: x[0]
	elif isinstance(a, sky.SkyStuff):
		if not isinstance(b, sky.SkyStuff):
			raise RuntimeError("Tried to interpolate SkyStuff with non-SkyStuff!")
		a = (a.game_angle, a.game_y_offset, a.game_d_offset, a.game_tilt[0], a.game_tilt[1], a.game_tilt[2], a.game_tilt[3], a.t3_angle)
		b = (b.game_angle, b.game_y_offset, b.game_d_offset, b.game_tilt[0], b.game_tilt[1], b.game_tilt[2], b.game_tilt[3], b.t3_angle)
		return_interpreter = lambda x: sky.SkyStuff(x[0], x[1], x[2], (x[3], x[4], x[5], x[6]), x[7])
	
	if len(a) != len(b):
		raise RuntimeError("Non-equal-type tuples given to interpolate")
	if x <= 0.0:
		return a
	elif x >= 1.0:
		return b
	
	interpolator = None
	if mode == INTERP_MODE_LINEAR:
		interpolator = lambda i, j: i + (j-i)*x
	elif mode == INTERP_MODE_SMOOTHED:
		def smooth_interp(i, j):
			if x < 0.5:
				return i + (j-i)*2*x*x
			else:
				return i + (j-i)*(1 - (2*(1-x)*(1-x)))
		interpolator = smooth_interp
	elif mode == INTERP_MODE_LOG_DOWN:
		SCALING = 1000
		def log_down_interp(i, j):
			if x == 0.0:
				return 0.0
			else:
				return i + (j-i)*(max(0.0, (math.log(x)+SCALING)/SCALING))
		interpolator = log_down_interp
	
	r = []
	for n in range(len(a)):
		r.append(interpolator(a[n], b[n]))
	return return_interpreter(tuple(r))

def applyMatrix(point, matrix):
	"""Given a Point and an OpenGL matrix, returns the Point as transformed by the matrix."""
	pMatrix = (point[0], point[1], point[2], 1.0)
	r = [sum([pMatrix[i]*matrix[i][j] for i in range(4)]) for j in range(4)]
	return Point(r[0], r[1], r[2])

def applyTransverseMatrix(point, matrix):
	"""Given a Point and an OpenGL matrix, returns the Point as transformed by the transverse of the matrix."""
	pMatrix = (point[0], point[1], point[2], 1.0)
	r = [sum([pMatrix[i]*matrix[j][i] for i in range(4)]) for j in range(4)]
	return Point(r[0], r[1], r[2])

def rev2rad(ang):
	"""Converts an angle in cw revolutions to ccw radians.
	
	This is mostly used when handing angles to ODE.
	"""
	return -2.0 * ang * math.pi

def rad2rev(ang):
	"""Converts an angle in radians to revolutions."""
	return ang/(math.pi*2.0)

def rev2deg(ang):
	"""Converts an angle in cw revolutions to cw degrees.

	This is mostly used when handing angles to OpenGL.
	"""
	return 360 * ang

def deg2rev(ang):
	"""Converts an angle in degrees to revolutions."""
	return ang/360

def min_ang_diff(src, dest):
	"""Returns the shortest angular distance between two angles (in revolutions).

	By "shortest", I mean that this will return whichever is lesser out of
	clockwise or counter-clockwise."""
	dist = (dest - src) % 1
	if 1-dist < dist:
		dist = -(1-dist)
	return dist

def cap_ang_diff(ang, max):
	"""Reduces an angle offset to be less than a maximum, maintaining sign."""
	if abs(ang) > max:
		if ang > 0:
			return max
		else:
			return -max
	else:
		return ang

def anchored_joint(joint_type, obj1, anchor = None, obj2 = None):
	"""Creates a new joint of the given type between two GameObjs, calls setAnchor on it.

	Anchor is given as a point offset relative to the position of obj1.
	
	If obj2 is unspecified, then the joint is created between obj1 and the static environment.
	Either way, object(s) must be correctly positioned before this method is called.
	"""
	if anchor == None: anchor = Point(0, 0)
	joint = joint_type(app.odeworld)
	if obj2 != None:
		joint.attach(obj1.body, obj2.body)
	else:
		joint.attach(obj1.body, ode.environment)
	joint.setAnchor((obj1.pos[0] + anchor[0], obj1.pos[1] + anchor[1], 0))
	return joint

def sphere_body(mass, radius):
	"""Creates an ODE body which is a sphere of the given mass and radius.
	
	It will be given a body_type data attribute set to "sphere".
	It will be given a radius attribute set to the given radius argument.
	"""
	
	body = ode.Body(app.odeworld)
	omass = ode.Mass()
	omass.setSphereTotal(mass, radius)
	body.setMass(omass)
	body.radius = radius
	body.body_type = "sphere"
	return body

def ccyl_body(mass, radius, total_length):
	"""Creates an ODE body which is a capped cylinder of the given mass, radius, and total length.

	The total length must be greater than twice the radius.
	
	The cylinder will be oriented down the body's Z axis, and centered at the body's center of mass.
	
	It will be given a body_type data attribute set to "ccyl".
	It will be given radius and total_length attributes set to the given arguments.
	"""
	
	body = ode.Body(app.odeworld)
	omass = ode.Mass()
	omass.setCappedCylinderTotal(mass, 3, radius, total_length - radius*2)
	body.setMass(omass)
	body.radius, body.total_length = radius, total_length
	body.body_type = "ccyl"
	return body

