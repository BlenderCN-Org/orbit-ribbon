from __future__ import division

import pygame, os, ode, math

import app, collision
from geometry import *

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

