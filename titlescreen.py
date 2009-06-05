from __future__ import division

import pygame
from OpenGL.GL import *
from OpenGL.GLU import *

import app, resman, camera, sky
from geometry import *
from util import *

TSMODE_PRE_PRE_MAIN, TSMODE_PRE_MAIN, TSMODE_MAIN, TSMODE_PRE_AREA, TSMODE_AREA, TSMODE_PRE_MISSION, TSMODE_MISSION, TSMODE_PRE_GAMEPLAY = range(8)
PRE_PRE_MAIN_MILLISECS = 1500
PRE_MAIN_MILLISECS = 3000
PRE_AREA_MILLISECS = 2000
AREA_FADEIN_MILLISECS = 300
AREA_FADEOUT_MILLISECS = 200
PRE_MISSION_MILLISECS = 500
PRE_GAMEPLAY_MILLISECS = 5000

class _TitleScreenCamera(camera.Camera):
	def __init__(self, manager):
		self.manager = manager
		self.mission_cam = None # Set externally by TitleScreenManager
		self.gameplay_cam = None # Set externally by TitleScreenManager
		
		# Fixed camera positions, written assuming that SkyStuff is in default state (which it is, prior to choosing an area)
		self.pre_main_cam = camera.FixedCamera(
			position = Point(0, sky.GOLD_DIST*20, sky.GOLD_DIST),
			target = Point(0, 0, sky.GOLD_DIST),
			up_vec = Point(0, 0, 1)
		)
		self.main_cam = camera.FixedCamera(
			position = Point(0, sky.GOLD_DIST*0.6, -sky.GOLD_DIST*1.5),
			target = Point(0, sky.GOLD_DIST*0.4, sky.GOLD_DIST),
			up_vec = Point(0, 1, 0)
		)
		self.area_cam = camera.FixedCamera(
			position = Point(0, sky.GOLD_DIST*2.8, sky.GOLD_DIST),
			target = Point(0, 0, sky.GOLD_DIST),
			up_vec = Point(0, 0, 1)
		)
	
	def get_camvals(self):
		# TODO : To add some juiciness, allow player to spin around the ring with controller during most modes
		if self.manager._tsmode == TSMODE_PRE_PRE_MAIN:
			return self.pre_main_cam.get_camvals()
		elif self.manager._tsmode == TSMODE_PRE_MAIN:
			return interpolate(
				self.pre_main_cam.get_camvals(),
				self.main_cam.get_camvals(),
				(pygame.time.get_ticks() - self.manager._tstart)/PRE_MAIN_MILLISECS,
				INTERP_MODE_SMOOTHED
			)
		elif self.manager._tsmode == TSMODE_MAIN:
			return self.main_cam.get_camvals()
		elif self.manager._tsmode == TSMODE_PRE_AREA:
			return interpolate(
				self.main_cam.get_camvals(),
				self.area_cam.get_camvals(),
				(pygame.time.get_ticks() - self.manager._tstart)/PRE_AREA_MILLISECS,
				INTERP_MODE_SMOOTHED
			)
		elif self.manager._tsmode == TSMODE_AREA:
			return self.area_cam.get_camvals()
		elif self.manager._tsmode == TSMODE_PRE_MISSION:
			return interpolate(
				self.area_cam.get_camvals(),
				self.mission_cam.get_camvals(),
				(pygame.time.get_ticks() - self.manager._tstart)/PRE_AREA_MILLISECS,
				INTERP_MODE_SMOOTHED
			)
		elif self.manager._tsmode == TSMODE_MISSION:
			return self.mission_cam.get_camvals()
		elif self.manager._tsmode == TSMODE_PRE_GAMEPLAY:
			# To transition to gameplay mode, travel from mission camera to the title camera, then from there to gameplay camera
			t = (pygame.time.get_ticks() - self.manager._tstart)/PRE_GAMEPLAY_MILLISECS
			if t < 0.5:
				return interpolate(
					self.mission_cam.get_camvals(),
					self.main_cam.get_camvals(),
					t*2,
					INTERP_MODE_SMOOTHED
				)
			else:
				return interpolate(
					self.main_cam.get_camvals(),
					self.gameplay_cam.get_camvals(),
					(t - 0.5)*2,
					INTERP_MODE_LOG_DOWN
				)


class TitleScreenManager:
	"""Manages the behavior of the pre-gameplay virtual interface.
	
	Data attributes:
	camera - A camera.Camera object that should be used to control the 3D viewpoint while the title screen is active.
	cur_area - The currently selected AreaDesc, or None if no area has been selected. Do not set externally.
	"""
	
	def __init__(self):
		self._title_tex = resman.Texture("title.png")
		self._selsquare_active_tex = resman.Texture("selsquare_active.png")
		self._selsquare_inactive_tex = resman.Texture("selsquare_inactive.png")
		self.camera = _TitleScreenCamera(self)
		#self._set_mode(TSMODE_PRE_PRE_MAIN)
		self._set_mode(TSMODE_AREA)
		self.cur_area = None
		self._cur_area_start = None # Tick time at which cur_area was set.
	
	def _set_mode(self, new_mode):
		self._tsmode = new_mode
		self._tstart = pygame.time.get_ticks()
	
	def _draw_title_logo(self, alpha):
		top_margin = app.winsize[1]/100
		left_margin = app.winsize[0]/5
		right_margin = app.winsize[0]/4
		title_ul = (left_margin, top_margin)
		title_lr = (app.winsize[0] - right_margin, top_margin + self._title_tex.size[1]*(app.winsize[0] - left_margin - right_margin)/self._title_tex.size[0])
		glColor(1, 1, 1, alpha)
		glEnable(GL_TEXTURE_2D)
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE)
		glBindTexture(GL_TEXTURE_2D, self._title_tex.glname)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
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
	
	def draw(self):
		if self._tsmode == TSMODE_PRE_PRE_MAIN:
			### Transition from blank screen into distant view of Smoke Ring
			doneness = (pygame.time.get_ticks() - self._tstart)/PRE_PRE_MAIN_MILLISECS
			app.fade_color = (0, 0, 0, 1 - doneness)
			if doneness >= 1.0:
				self._set_mode(TSMODE_PRE_MAIN)
		elif self._tsmode == TSMODE_PRE_MAIN:
			### Transition from pre-pre-main into the main screen showing the title logo
			doneness = (pygame.time.get_ticks() - self._tstart)/PRE_MAIN_MILLISECS
			if doneness >= 0.5:
				self._draw_title_logo(doneness - 0.5)
			if doneness >= 1.0:
				self._set_mode(TSMODE_MAIN)
		elif self._tsmode == TSMODE_MAIN:
			### Draw/handle events for the main title screen interface
			self._draw_title_logo(0.5)
			for e in app.events:
				if e.type == pygame.KEYDOWN and e.key == pygame.K_SPACE:
					# Handle input events to proceed into area selection
					self._set_mode(TSMODE_PRE_AREA)
		elif self._tsmode == TSMODE_PRE_AREA:
			### Transition from main title screen to area selection screen
			doneness = (pygame.time.get_ticks() - self._tstart)/PRE_AREA_MILLISECS
			self._draw_title_logo(max(0.0, 0.5 - doneness))
			if doneness >= 1.0:
				self._set_mode(TSMODE_AREA)
		elif self._tsmode == TSMODE_AREA:
			### Draw/handle events for the area selection interface
			selsize = app.winsize[1]/20
			if self.cur_area is None:
				fade = (pygame.time.get_ticks() - self._tstart)/AREA_FADEIN_MILLISECS
			else:
				fade = max(0.0, 1.0 - (pygame.time.get_ticks() - self._cur_area_tstart)/AREA_FADEOUT_MILLISECS)
			glColor(1, 1, 1, fade)
			for area in app.areas:
				# FIXME: This distance should be calculated from actual projection, not reckoned like this. Probably good enough for gov't work, though.
				d = (app.winsize[1]/2)*0.855 * (1 + area.sky_stuff.game_d_offset/sky.GOLD_DIST)
				ang = rev2rad(area.sky_stuff.game_angle)
				pos = Point(app.winsize[0]/2, app.winsize[1]/2, 0) + Point(d*math.sin(ang), d*math.cos(ang), 0)
				glEnable(GL_TEXTURE_2D)
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE)
				glBindTexture(GL_TEXTURE_2D, self._selsquare_inactive_tex.glname)
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
				glBegin(GL_QUADS)
				glTexCoord2f(0.0, 1.0)
				glVertex2f(pos[0] - selsize, pos[1] - selsize)
				glTexCoord2f(1.0, 1.0)
				glVertex2f(pos[0] + selsize, pos[1] - selsize)
				glTexCoord2f(1.0, 0.0)
				glVertex2f(pos[0] + selsize, pos[1] + selsize)
				glTexCoord2f(0.0, 0.0)
				glVertex2f(pos[0] - selsize, pos[1] + selsize)
				glEnd()
				glDisable(GL_TEXTURE_2D)
			if self.cur_area is None:
				for e in app.events:
					if e.type == pygame.KEYDOWN and e.key == pygame.K_SPACE:
						self.cur_area = area
						self._cur_area_tstart = pygame.time.get_ticks()
						ang = rev2rad(area.sky_stuff.game_angle)
						d = sky.GOLD_DIST + area.sky_stuff.game_d_offset
						area_loc = Point(0, 0, sky.GOLD_DIST) - Point(d*math.sin(ang), 0, d*math.cos(ang))
						self.camera.mission_cam = camera.FixedCamera(
							position = area_loc + Point(0, sky.GOLD_DIST/10, 0),
							target = area_loc,
							up_vec = Point(0, 0, 1)
						)
			else:
				if fade == 0.0:
					# Once we've faded out the UI, transition to mission selection
					self._set_mode(TSMODE_PRE_MISSION)
		elif self._tsmode == TSMODE_PRE_MISSION:
			doneness = (pygame.time.get_ticks() - self._tstart)/PRE_AREA_MILLISECS
			if doneness >= 1.0:
				self._set_mode(TSMODE_MISSION)
		elif self._tsmode == TSMODE_MISSION:
			### Draw/handle events for the mission selection interface
			for e in app.events:
				if e.type == pygame.KEYDOWN and e.key == pygame.K_SPACE:
					# Transition to gameplay mode
					area = self.cur_area
					app.objects = area.objects
					self.camera.gameplay_cam = camera.FollowCamera(
						target_obj = app.objects[0]
					)
					self._set_mode(TSMODE_PRE_GAMEPLAY)
		elif self._tsmode == TSMODE_PRE_GAMEPLAY:
			doneness = (pygame.time.get_ticks() - self._tstart)/PRE_GAMEPLAY_MILLISECS
			area = self.cur_area
			if doneness < 0.5:
				app.sky_stuff = interpolate(
					sky.SkyStuff(),
					area.sky_stuff,
					doneness*2,
					INTERP_MODE_SMOOTHED
				)
			if doneness >= 1.0:
				# Begin gameplay mode
				app.sky_stuff = area.sky_stuff
				app.player_camera = self.camera.gameplay_cam
				app.mode = app.MODE_GAMEPLAY
