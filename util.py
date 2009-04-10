from __future__ import division

import pygame, os, ode, math

import app, collision
from geometry import *

def rev2rad(ang):
	"""Converts an angle in cw revolutions to ccw radians.
	
	This is mostly used when handing angles to ODE.
	"""
	return -2.0 * ang * math.pi

def rev2deg(ang):
	"""Converts an angle in cw revolutions to cw degrees.

	This is mostly used when handing angles to OpenGL.
	"""
	return 360 * ang

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

def sphere_body(density, radius):
	"""Creates an ODE body which is a sphere of the given density and radius.
	
	It will be given a body_type data attribute set to "sphere".
	It will be given radius and density data attributes, set to the given arguments.
	"""
	
	body = ode.Body(app.odeworld)
	omass = ode.Mass()
	omass.setSphere(density, radius)
	body.setMass(omass)
	body.radius = radius
	body.density = density
	body.body_type = "sphere"
	return body
