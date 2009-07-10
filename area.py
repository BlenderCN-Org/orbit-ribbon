from __future__ import division

class AreaDesc:
	"""Describes an area (a level) that the player can go to. Each area has one or more missions.
	
	Data attributes:
	name - A string with the internal name of the area (i.e. "A01-Base")
	player_name - A string with the player-visible name of the area (i.e. "Quaternion Jungle")
	sky_stuff - The sky.SkyStuff object for this area, which is used to position it in the Smoke Ring.
	objects - A sequence of GameObjects that apply to all missions in this area; the basic geometry of the level.
	"""
	def __init__(self, name, player_name, sky_stuff, objects):
		self.name = name
		self.player_name = player_name
		self.sky_stuff = sky_stuff
		self.objects = objects
