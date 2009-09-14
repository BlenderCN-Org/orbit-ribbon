from __future__ import division

import pygame, os

import app, resman, camera, sky, console, inputs
from geometry import *
from util import *
from gl import *

TSMODE_PRE_PRE_MAIN, TSMODE_PRE_MAIN, TSMODE_MAIN, TSMODE_PRE_AREA, TSMODE_AREA, TSMODE_PRE_MISSION, TSMODE_MISSION, TSMODE_PRE_GAMEPLAY = range(8)
PRE_PRE_MAIN_MILLISECS = 1500
PRE_MAIN_MILLISECS = 3000
PRE_AREA_MILLISECS = 2000
AREA_FADEIN_MILLISECS = 300
AREA_FADEOUT_MILLISECS = 200
MISSION_FADEIN_MILLISECS = 300
MISSION_FADEOUT_MILLISECS = 200
PRE_MISSION_MILLISECS = 500
PRE_GAMEPLAY_MILLISECS = 10000

# FIXME This entire damn thing needs to be refactored into some general purpose UI kit with transitions that can go forwards or back, and overlap
# Probably should do that around the time I decide to implement the options dialogue or the pause screen menu
# While I'm at it, should probably also take the opportunity to improve the mess of globals lying around in app's namespace

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


class Animation:
	"""Displays a 2D animation.
	
	The animation will not skip frames.
	
	Data attributes:
	textures - A sequence of resman.Textures to draw
	delay - How many milliseconds should pass between frames
	"""
	def __init__(self, dirname, delay):
		"""Creates a new Animation using the sequentially numbered PNGs found in the given directory."""
		self.textures = []
		for fn in sorted(os.listdir(os.path.join(app.APP_DIR, 'images', dirname))):
			if fn.endswith(".png"):
				self.textures.append(resman.Texture(os.path.join(dirname, fn)))

		self.delay = delay
		self._cur_frame = 0 # Index of the current frame
		self._last_frame_time = None # Tick time at which the previous frame was displayed, None if no frame displayed yet
	
	def draw(self, left, top, width, height):
		"""Draws the current at the given position and size.
		
		OpenGL must be in a 2D drawing mode before this is called.
		"""
		now = pygame.time.get_ticks()
		if self._last_frame_time is None:
			self._last_frame_time = now
		elif now - self._last_frame_time > self.delay:
			self._cur_frame += 1
			if self._cur_frame > (len(self.textures)-1):
				self._cur_frame = 0
			self._last_frame_time = now
		self.textures[self._cur_frame].draw_2d(left, top, width, height)


class TitleScreenManager:
	"""Manages the behavior of the pre-gameplay virtual interface.
	
	Data attributes:
	camera - A camera.Camera object that should be used to control the 3D viewpoint while the title screen is active.
	cur_sel - An index (from 0) indicating which area, mission, etc. the cursor is selecting, in relevant title screen modes
	"""
	
	def __init__(self):
		self._title_tex = resman.Texture("title.png")
		self._selsquare_active_tex = resman.Texture("selsquare_active.png")
		self._selsquare_inactive_tex = resman.Texture("selsquare_inactive.png")
		self.camera = _TitleScreenCamera(self)
		self._set_mode(TSMODE_PRE_PRE_MAIN)
		self.cur_sel = None
	
	def _set_mode(self, new_mode):
		self._tsmode = new_mode
		self._tstart = pygame.time.get_ticks()
		self.cur_sel = None
	
	def _draw_title_logo(self, alpha):
		top_margin = app.winsize[1]/100
		left_margin = app.winsize[0]/5
		right_margin = app.winsize[0]/4
		width = (app.winsize[0] - right_margin) - left_margin
		height = self._title_tex.size[1]*(app.winsize[0] - left_margin - right_margin)/self._title_tex.size[0]
		glColor(1, 1, 1, alpha)
		self._title_tex.draw_2d(left_margin, top_margin, width, height)
	
	def draw(self):
		skipping = inputs.INTENT_UI_CONFIRM in app.event_intents
		
		if self._tsmode == TSMODE_PRE_PRE_MAIN:
			### Transition from blank screen into distant view of Smoke Ring
			doneness = (pygame.time.get_ticks() - self._tstart)/PRE_PRE_MAIN_MILLISECS
			if skipping:
				doneness = 1
			app.fade_color = (0, 0, 0, 1 - doneness)
			if doneness >= 1.0:
				if skipping:
					self._set_mode(TSMODE_MAIN)
				else:
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
			if inputs.INTENT_UI_CONFIRM in app.event_intents:
				# Proceed into area selection
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
			# FIXME: Display information about the area under consideration, at least the visible name
			selsize = app.winsize[1]/20
			if app.cur_area is None:
				fade = (pygame.time.get_ticks() - self._tstart)/AREA_FADEIN_MILLISECS
			else:
				fade = max(0.0, 1.0 - (pygame.time.get_ticks() - app.cur_area_tstart)/AREA_FADEOUT_MILLISECS)
			glColor(1, 1, 1, fade)
			for area in app.ore_man.areas.itervalues():
				# FIXME: This distance should be calculated from actual projection, not reckoned like this. Probably good enough for gov't work, though.
				d = (app.winsize[1]/2)*0.855 * (1 + area.sky_stuff.game_d_offset/sky.GOLD_DIST)
				ang = rev2rad(area.sky_stuff.game_angle)
				pos = Point(app.winsize[0]/2, app.winsize[1]/2, 0) + Point(d*math.sin(ang), d*math.cos(ang), 0)
				glEnable(GL_TEXTURE_2D)
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE)
				glBindTexture(GL_TEXTURE_2D, self._selsquare_inactive_tex.glname)
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
			if app.cur_area is None:
				if inputs.INTENT_UI_CONFIRM in app.event_intents:
					app.init_area("A01") # FIXME: Allow the user to select which area they want to go to
					app.sky_stuff = sky.SkyStuff() # Override the sky choice made by the area loader
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
				self.cur_sel = 0
		elif self._tsmode == TSMODE_MISSION:
			### Draw/handle events for the mission selection interface
			if app.cur_mission is None:
				fade = (pygame.time.get_ticks() - self._tstart)/MISSION_FADEIN_MILLISECS
			else:
				fade = max(0.0, 1.0 - (pygame.time.get_ticks() - app.cur_mission_tstart)/MISSION_FADEOUT_MILLISECS)
			mission_names = sorted(app.cur_area.missions.keys())
			y = 200
			for i, name in enumerate(mission_names):
				rect = pygame.Rect(200, y, 400, 35)
				obox = console.OutputBox(rect)
				obox.append("%u: %s" % (i+1, app.cur_area.missions[name].visible_name))
				obox.draw()
				if self.cur_sel == i:
					glColor(1, 1, 1, fade) # FIXME This is actually 99% useless since OutputBox sets its own colors. So I put it here to fade cursor box.
					glBegin(GL_LINES)
					glVertex2fv(rect.topleft)
					glVertex2fv(rect.topright)
					glVertex2fv(rect.bottomright)
					glVertex2fv(rect.bottomleft)
					glEnd()
				y += 50
			if app.cur_mission is None:
					if inputs.INTENT_UI_CONFIRM in app.event_intents:
						# Transition to pre-gameplay mode on the currently selected mission
						mname = mission_names[self.cur_sel]
						app.init_mission(mname)
						# Override the camera set by init_mission, but keep it for later when we transition to actual gameplay mode
						self.camera.gameplay_cam = app.player_camera
						app.player_camera = self.camera
					elif inputs.INTENT_UI_Y in app.event_intents:
						# Move the cursor up or down
						val = app.input_man.intent_channels[inputs.INTENT_UI_Y].value()
						if val < 0 and self.cur_sel > 0:
							self.cur_sel -= 1
						elif val > 0 and self.cur_sel < (len(mission_names)-1):
							self.cur_sel += 1
			else:
				if fade == 0.0:
					# Once we've faded out the UI, transition to pre-gameplay
					self._set_mode(TSMODE_PRE_GAMEPLAY)
		elif self._tsmode == TSMODE_PRE_GAMEPLAY:
			doneness = (pygame.time.get_ticks() - self._tstart)/PRE_GAMEPLAY_MILLISECS
			if skipping:
				doneness = 1
			if doneness < 0.3:
				app.sky_stuff = interpolate(
					sky.SkyStuff(),
					app.cur_area.sky_stuff,
					doneness/0.3,
					INTERP_MODE_SMOOTHED
				)
			else:
				app.sky_stuff = app.cur_area.sky_stuff
			if doneness >= 1.0:
				# Begin gameplay mode
				app.player_camera = self.camera.gameplay_cam
				app.mode = app.MODE_GAMEPLAY
