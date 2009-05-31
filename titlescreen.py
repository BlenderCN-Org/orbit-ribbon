from __future__ import division

import pygame
from OpenGL.GL import *

import app, resman, camera, sky
from geometry import *
from util import *

TSMODE_PRE_MAIN, TSMODE_MAIN, TSMODE_PRE_AREA, TSMODE_AREA, TSMODE_PRE_MISSION, TSMODE_MISSION, TSMODE_PRE_GAMEPLAY = range(7)
PRE_MAIN_MILLISECS = 2000
PRE_AREA_MILLISECS = 500
PRE_MISSION_MILLISECS = 500
PRE_GAMEPLAY_MILLISECS = 2000

class _TitleScreenCamera(camera.Camera):
	def __init__(self, manager):
		self.manager = manager
		
		# Fixed camera positions, written assuming that SkyStuff is in default state (which it is, prior to choosing an area)
		self.pre_main_cam = camera.FixedCamera(
			position = Point(0, sky.GOLD_DIST*20, -sky.GOLD_DIST*2),
			target = Point(0, 0, 0),
			up_vec = Point(0, 0, 1)
		)
		self.main_cam = camera.FixedCamera(
			position = Point(0, sky.GOLD_DIST*1.8, -sky.GOLD_DIST*4.2),
			target = Point(0, sky.GOLD_DIST*1.5, 0),
			up_vec = Point(0, 1, 0)
		)
	
	def get_camvals(self):
		if self.manager._tsmode == TSMODE_PRE_MAIN:
			x = interpolate(
				self.pre_main_cam.get_camvals(),
				self.main_cam.get_camvals(),
				(pygame.time.get_ticks() - self.manager._tstart)/PRE_MAIN_MILLISECS,
				INTERP_MODE_LINEAR
			)
			return x
		elif self.manager._tsmode == TSMODE_MAIN:
			return self.main_cam.get_camvals()
		elif self.manager._tsmode == TSMODE_PRE_AREA:
			pass
		elif self.manager._tsmode == TSMODE_AREA:
			pass
		elif self.manager._tsmode == TSMODE_PRE_MISSION:
			pass
		elif self.manager._tsmode == TSMODE_MISSION:
			pass
		elif self.manager._tsmode == TSMODE_PRE_GAMEPLAY:
			pass


class TitleScreenManager:
	"""Manages the behavior of the pre-gameplay virtual interface.
	
	Data attributes:
	camera - A camera.Camera object that should be used to control the 3D viewpoint while the title screen is active.
	"""
	
	def __init__(self):
		self._title_tex = resman.Texture("title.png")
		self.camera = _TitleScreenCamera(self)
		self._set_mode(TSMODE_PRE_MAIN)
	
	def _set_mode(self, new_mode):
		self._tsmode = new_mode
		self._tstart = pygame.time.get_ticks()
	
	def draw(self):
		if self._tsmode == TSMODE_PRE_MAIN:
			doneness = (pygame.time.get_ticks() - self._tstart)/PRE_MAIN_MILLISECS
			if doneness >= 1.0:
				self._set_mode(TSMODE_MAIN)
		if self._tsmode == TSMODE_MAIN:
			### Draw the main title screen interface
			
			# Draw the title logo
			top_margin = app.winsize[1]/60
			left_margin = app.winsize[0]/40
			right_margin = app.winsize[0]/10
			title_ul = (left_margin, top_margin)
			title_lr = (app.winsize[0] - right_margin, top_margin + self._title_tex.size[1]*(app.winsize[0] - left_margin - right_margin)/self._title_tex.size[0])
			glEnable(GL_TEXTURE_2D)
			glBindTexture(GL_TEXTURE_2D, self._title_tex.glname)
			glBegin(GL_QUADS)
			glTexCoord2f(0.0, 1.0)
			glVertex2f(title_ul[0], title_ul[1])
			glTexCoord2f(1.0, 1.0)
			glVertex2f(title_lr[0], title_ul[1])
			glTexCoord2f(1.0, 0.0)
			glVertex2f(title_lr[0], title_lr[1])
			glTexCoord2f(0.0, 0.0)
			glVertex2f(title_ul[0], title_lr[1])
			glEnd()
			glDisable(GL_TEXTURE_2D)
			
			for e in app.events:
				if e.type == pygame.KEYDOWN and e.key == pygame.K_SPACE:
					app.set_game_mode(app.MODE_GAMEPLAY)
		elif self._tsmode == TSMODE_AREA:
			### Draw the area selection interface
			pass
		elif self._tsmode == TSMODE_MISSION:
			### Draw the mission selection interface
			pass
