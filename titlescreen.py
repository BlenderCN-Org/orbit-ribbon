from __future__ import division

import pygame
from OpenGL.GL import *
from OpenGL.GLU import *

import app, resman, camera, sky, avatar
from geometry import *
from util import *

TSMODE_PRE_PRE_MAIN, TSMODE_PRE_MAIN, TSMODE_MAIN, TSMODE_PRE_AREA, TSMODE_AREA, TSMODE_PRE_MISSION, TSMODE_MISSION, TSMODE_PRE_GAMEPLAY = range(8)
PRE_PRE_MAIN_MILLISECS = 1500
PRE_MAIN_MILLISECS = 3000
PRE_AREA_MILLISECS = 2000
AREA_FADEIN_MILLISECS = 300
AREA_FADEOUT_MILLISECS = 200
PRE_MISSION_MILLISECS = 500
PRE_GAMEPLAY_MILLISECS = 10000

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
			# Describes the steps we will go through to approach gameplay camera
			# Tuples are: start point inclusive, end point exclusive, cameras to transition between, and interpolation mode
			midpoint_1 = interpolate(self.main_cam.get_camvals(), self.gameplay_cam.get_camvals(), 0.92, INTERP_MODE_LINEAR)
			midpoint_2 = interpolate(self.main_cam.get_camvals(), self.gameplay_cam.get_camvals(), 0.9999, INTERP_MODE_LINEAR)
			stages = (
				(0.0, self.mission_cam.get_camvals(), self.main_cam.get_camvals(), INTERP_MODE_SMOOTHED),
				(0.3, self.main_cam.get_camvals(), midpoint_1, INTERP_MODE_LINEAR),
				(0.5, midpoint_1, midpoint_2, INTERP_MODE_LINEAR),
				(0.7, midpoint_2, self.gameplay_cam.get_camvals(), INTERP_MODE_SMOOTHED),
			)
			t = (pygame.time.get_ticks() - self.manager._tstart)/PRE_GAMEPLAY_MILLISECS
			for n, (start_pt, start_camvals, end_camvals, interp_mode) in enumerate(stages):
				end_pt = 1.0
				if n+1 < len(stages):
					end_pt = stages[n+1][0]
				if n+1 == len(stages) or t < end_pt:
					return interpolate(start_camvals, end_camvals, (t-start_pt)/(end_pt-start_pt), interp_mode)
			else:
				return stages[-1][2]


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
		self._set_mode(TSMODE_PRE_PRE_MAIN)
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
		skipping = False
		for e in app.events:
			if e.type == pygame.KEYDOWN and e.key in (pygame.K_ESCAPE, pygame.K_SPACE):
				skipping = True
				break
		
		if self._tsmode == TSMODE_PRE_PRE_MAIN:
			### Transition from blank screen into distant view of Smoke Ring
			doneness = (pygame.time.get_ticks() - self._tstart)/PRE_PRE_MAIN_MILLISECS
			if skipping:
				doneness = 1
			app.fade_color = (0, 0, 0, 1 - doneness)
			if doneness >= 1.0:
				self._set_mode(TSMODE_PRE_MAIN)
		elif self._tsmode == TSMODE_PRE_MAIN:
			### Transition from pre-pre-main into the main screen showing the title logo
			doneness = (pygame.time.get_ticks() - self._tstart)/PRE_MAIN_MILLISECS
			if skipping:
				doneness = 1
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
			if skipping:
				doneness = 1
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
			for area in app.ore_man.areas.itervalues():
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
						app.objects = self.cur_area.objects
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
			if skipping:
				doneness = 1
			if doneness >= 1.0:
				self._set_mode(TSMODE_MISSION)
		elif self._tsmode == TSMODE_MISSION:
			### Draw/handle events for the mission selection interface
			for e in app.events:
				if e.type == pygame.KEYDOWN and e.key == pygame.K_SPACE:
					# Transition to pre-gameplay mode
					app.objects = self.cur_area.objects + self.cur_area.missions["A01-M01"].objects # FIXME Testing
					app.mission_control = self.cur_area.missions["A01-M01"].mission_control # FIXME More testing
					target_obj = None
					for obj in app.objects:
						if isinstance(obj, avatar.Avatar):
							target_obj = obj
							break
					if target_obj is None:
						raise RuntimeError("Unable to find Avatar to target camera towards for entering gameplay mode!")
					self.camera.gameplay_cam = camera.FollowCamera(
						target_obj = target_obj
					)
					self._set_mode(TSMODE_PRE_GAMEPLAY)
		elif self._tsmode == TSMODE_PRE_GAMEPLAY:
			doneness = (pygame.time.get_ticks() - self._tstart)/PRE_GAMEPLAY_MILLISECS
			if skipping:
				doneness = 1
			if doneness < 0.3:
				app.sky_stuff = interpolate(
					sky.SkyStuff(),
					self.cur_area.sky_stuff,
					doneness/0.3,
					INTERP_MODE_SMOOTHED
				)
			else:
				app.sky_stuff = self.cur_area.sky_stuff
			if doneness >= 1.0:
				# Begin gameplay mode
				app.player_camera = self.camera.gameplay_cam
				app.mode = app.MODE_GAMEPLAY
