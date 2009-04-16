from __future__ import division

import ode

import app, util
from geometry import *

def collision_cb(contactgroup, geom1, geom2):
	"""Callback function to the collide method."""
	
	#Get collision props objects if they exist
	g1_coll_props = getattr(geom1, "coll_props", None)
	g2_coll_props = getattr(geom2, "coll_props", None)
	
	#If both geoms either have collision properties or are spaces, then perform a collision
	if g1_coll_props != None and g2_coll_props != None:
		# Collision between two non-spaces, handle it directly
		g1_coll_props.handle_collision(geom1, geom2, contactgroup)
	elif (geom1.isSpace() or g1_coll_props != None) and (geom2.isSpace() or g2_coll_props != None):
		# At least one of these two objects is a space, descend recursively
		ode.collide2(geom1, geom2, contactgroup, collision_cb)

class Collision:
	"""Describes a collision that took place between two ODE geoms.
	
	This object is meant to be kept in sub-arrays of the app.collisions dictionary. That dictionary
	is keyed by the id of a geom, so this object only needs to keep track of the 2nd geom that the
	key geom collided with.
	
	Data attributes:
	geom -- The ODE geom that the key geom collided with.
	pos -- An array of all the points where the objects collide; weirdly-shaped objects can collide in several spots.
	"""
	
	def __init__(self, geom, cpoints):
		self.geom = geom
		self.cpoints = cpoints[:]
	
	def avg_pos(self):
		"""Returns an average Point of all the cpoints, in absolute coordinates."""
		r = Point()
		for p in cpoints:
			r += p
		r /= len(self.cpoints)
		return r

class Props:
	"""Defines the collision properties of some object.
	
	You must assign one of these to the "coll_props" attribute of any non-Space Geom
	in the system if you expect the engine to pay attention to collisions involving
	those Geoms. Do not assign "coll_props" attributes to Spaces.
	
	You can disable the generation of contact joints by setting intersec_push
	to False. This is useful if you're planning to use the collision engine for logic
	rather than to represent a physical object. To do that, in the step method
	of your GameObj derivative class, check for the id of your object's geom
	in app.collisions, and you can find any other geom it collided with.
	
	The basis of the intersec_pri stuff it that is that two objects that
	collide physically may require either a ContactJoint between them if they've
	got equal collision priority, or may require a ContactJoint between the world
	and the one with lesser priority. There's no reason I can think of now
	to have any more than two layers of priority, but what the heck. The point
	of this system is to allow things like moving platforms that have bodies
	and geoms but cannot be pushed around by the player, although they can push
	the player around.
	
	Data attributes:
	intersec_push -- If True, creates contact joints at intersections to push this object, the other, or both away.
		Both objects must have this flag on for any intersection prevention to occur.
	intersec_pri -- The numeric collision priority for intersection-stopping (defaults to 1).
	"""
	
	def __init__(self, intersec_push = True, intersec_pri = 1):
		self.intersec_push = intersec_push
		self.intersec_pri = intersec_pri
	
	def handle_collision(self, geom1, geom2, cjointgroup):
		"""Checks for a real collision between geom1 (which should be the geom that has this Props as a
		.coll_props data attribute) and some other geom, also with a valid .coll_props. If there is a
		collision, then entries are made in app.collisions. Furthermore, if both objects have intersec_push
		enabled in their coll_props, then contact joints are created to push the objects apart.
		
		For any given collision, this is only called once, so this method needs to handle
		both this object's reaction as well as the other's.
		
		Newly created contact joints are placed into cjointgroup."""
		
		contacts = ode.collide(geom1, geom2)
		if len(contacts) == 0:
			return
		
		# Add the collision to app.collisions
		cpoints = []
		for c in contacts:
			cpoints.append(Point(c.getContactGeomParams()[0][0], c.getContactGeomParams()[0][1], c.getContactGeomParams()[0][2]))
		
		for (a,b) in ((id(geom1), geom2), (id(geom2), geom1)):
			if not app.collisions.has_key(a):
				app.collisions[a] = [Collision(b, cpoints)]
			else:
				app.collisions[a].append(Collision(b, cpoints))
		
		# Create contact joints if appropriate
		if self.intersec_push and geom2.coll_props.intersec_push:
			for c in contacts:
				c.setMode(ode.ContactApprox1 | ode.ContactBounce)
				c.setBounce(0.5)
				c.setMu(5000)
				cjoint = ode.ContactJoint(app.odeworld, cjointgroup, c)
				#FIXME: Collision priority stuff doesn't work very well when higher priority object pushes
				if self.intersec_pri == geom2.coll_props.intersec_pri:
					#Push both objects away from each other
					cjoint.attach(geom1.getBody(), geom2.getBody())
				elif self.intersec_pri > geom2.coll_props.intersec_pri:
					#Push the other object, but not this one
					cjoint.attach(None, geom2.getBody())
				else:
					#Push this object, not the other one
					cjoint.attach(geom1.getBody(), None)
