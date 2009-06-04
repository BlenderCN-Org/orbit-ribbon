from __future__ import division

import app

class AreaDesc:
	"""Describes an area (that is, a "level") that the player can go to.
	
	Data attributes:
	sky_stuff - The sky.SkyStuff object for this area, which is used to position it in the Smoke Ring.
	objects - The contents of app.objects that apply to all missions in this area.
	"""
	def __init__(self, sky_stuff, objects):
		self.sky_stuff = sky_stuff
		self.objects = objects
