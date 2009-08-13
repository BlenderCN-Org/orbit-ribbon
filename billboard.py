from __future__ import division

import math

from util import *
from gl import *

# TODO : Probably should turn this into a Landscape module, where you have a LandscapeList filled with LandscapeObjects
# Each LandscapeObject would have the ability to be drawn at several detail levels, including meshes, billboards, points, and not at all
# There'd be a specific class for spherical landscape objects, since those are common in Orbit Ribbon and work similarly to each other
# There might also be classes for integral trees, for vehicles, for Voy and T3 with their halo glows, etc.


class Billboard:
	"""A texture which always points towards the camera, cheaply creating the impression
	of a three-dimensional object.
	
	These are intended to be members of a BillboardSet. You can draw() them explicitly,
	but it's not very efficient.
	
	Data attributes (read only):
	pos - The position of the Billboard.
	tex - The resman.Texture used to draw the billboard.
	width - The x and y length of the billboard.
	"""
	def __init__(self, pos, tex, width):
		"""Creates a Billboard with the given position, texture, and width."""
		self.pos = pos
		self.tex = tex
		self.width = width
	
	def draw(self, camPos):
		"""Draws the billboard facing the camera at the given location.
		
		Be sure to enable GL_TEXTURE_2D before calling this.
		"""
		camVec = -self.pos + camPos # Vector to the camera
		
		# If it's too far away compared to its size, don't bother with it
		# FIXME Come up with something meaningful here, not just this fudge value of 15
		if (self.width**2)/camVec.mag() < 15:
			return
		
		glBindTexture(GL_TEXTURE_2D, self.tex.glname)
		glPushMatrix()
		glTranslatef(*self.pos)
		
		# Rotate the billboard so that it faces the camera
		glRotatef(rev2deg(rad2rev(math.atan2(camVec[0], camVec[2]))), 0, 1, 0) # Rotate around y-axis...
		glRotatef(-rev2deg(rad2rev(math.atan2(camVec[1], math.sqrt(camVec[0]**2 + camVec[2]**2)))), 1, 0, 0) # Then tilt up/down on x-axis
		glBegin(GL_QUADS)
		glTexCoord2f(0.0, 0.0)
		glVertex3f(-self.width/2, -self.width/2, 0)
		glTexCoord2f(1.0, 0.0)
		glVertex3f( self.width/2, -self.width/2, 0)
		glTexCoord2f(1.0, 1.0)
		glVertex3f( self.width/2,  self.width/2, 0)
		glTexCoord2f(0.0, 1.0)
		glVertex3f(-self.width/2,  self.width/2, 0)
		glEnd()
		glPopMatrix()


class BillboardList(list):
	"""A list of Billboards which are drawn all at once.
	
	This class is optimized for the situation where the billboards are much farther
	away from the camera than the distances the camera itself travels as it moves around.
	That way, the billboards don't have to be recalculated, and the slight angle error
	caused by the camera's movement isn't visible to the player.
	
	This class caches various values once draw() has been called on it once. Please call
	clear_cache() if you change the contents of the BillboardList after calling draw(),
	so that the cache can be recalculated for the new set of Billboards.
	"""
	MAX_DIST_RATIO_DIFF = 0.001 #Ratio of (dist from new cam pos to old cam pos)/(dist from old cam pos to nearest billboard) at which we recalc cache
	
	def __init__(self, arg = None):
		if arg is None:
			super(BillboardList, self).__init__()
		else:
			super(BillboardList, self).__init__(arg)
		
		self._dispList = GLDisplayList()
		self.clear_cache()
	
	def clear_cache(self):
		"""Forces the next call to draw() to recalculate all cached values.
		
		Call this after altering the contents of the list.
		"""
		self._cacheOk = False
		self._closestBoardPos = None
		self._lastCamPos = None
		self._lastCamDist = None
	
	def draw(self, camPos):
		# If the new camera position is far enough away from the old one, force a cache recalculation
		if self._closestBoardPos is not None and camPos.dist_to(self._lastCamPos)/self._lastCamDist > self.MAX_DIST_RATIO_DIFF:
			self._cacheOk = False
		
		if self._cacheOk:
			self._dispList.play()
		else:
			candDist = None
			candPos = None
			for bb in self:
				dist = camPos.dist_to(bb.pos)
				if candPos is None or dist < candDist:
					candDist = dist
					candPos = bb.pos
			self._closestBoardPos = candPos.__copy__()
			self._lastCamPos = camPos.__copy__()
			self._lastCamDist = candDist
			
			self._dispList.begin_rec(GL_COMPILE_AND_EXECUTE)
			for bb in self:
				bb.draw(camPos)
			self._dispList.end_rec()
			
			self._cacheOk = True
