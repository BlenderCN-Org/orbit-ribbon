from __future__ import division

from OpenGL.GL import *
from OpenGL.GLUT import *

import app, resman, target

WIN_MESSAGE = "Complete!"
WIN_MESSAGE_TIME = 5.0
WIN_MESSAGE_FADE = 1.5

class MissionControl:
	"""Defines the parameters for this particular mission, such as what the win condition is.
	
	Data attributes:
	win_cond_func -- Specifies a function to be called each step. If the function
		returns True, then the player has won the mission, and the timer should be stopped.
	won_at -- If not None, this is the step number at which the player won the mission.
	timer_start_func -- If not None, specifies a function to be called each step. If the function
		returns True, then the timer starts. The timer stops when the win condition is
		reached. If None, then there is no timer.
	timer_start_at -- If not None, this is the step number at which timing began.
	"""
	
	def __init__(self, win_cond_func, timer_start_func = None):
		self.win_cond_func = win_cond_func
		self.won_at = None
		self.timer_start_func = timer_start_func
		self.timer_start_at = None
		
		self._start_sound = resman.SoundClip("/usr/share/sounds/gtkboard/machine_move.ogg")
		self._win_sound = resman.SoundClip("/usr/share/sounds/gtkboard/won.ogg")
	
	def step(self):
		"""To be called at the beginning of each step by app."""
		if self.timer_start_func is not None and self.timer_start_at is None and self.won_at is None:
			if self.timer_start_func():
				self._start_sound.snd.play()
				self.timer_start_at = app.totalsteps
		
		if self.won_at is None:
			if self.win_cond_func():
				self._win_sound.snd.play()
				self.won_at = app.totalsteps
	
	def draw(self):
		"""To be called during the 2D phase of drawing by app."""
		# If it's possible for the timer to start, draw the time elapsed in the upper left corner
		if self.timer_start_func is not None:
			diff = 0
			if self.timer_start_at is not None:
				end = app.totalsteps if self.won_at is None else self.won_at
				diff = end - self.timer_start_at
			seconds = diff/app.maxfps
			msg = "%.2fsec" % seconds
			glColor4f(0.0, 0.0, 0.0, 1.0)
			glRasterPos2f(app.winsize[0] - 100, 25)
			for c in msg:
				glutBitmapCharacter(GLUT_BITMAP_9_BY_15, ord(c))
		
		# Display a message for a short while after the player has won
		if self.won_at is not None:
			won_secs = (app.totalsteps - self.won_at)/app.maxfps
			if won_secs < WIN_MESSAGE_TIME:
				msg = WIN_MESSAGE
				alpha = 1.0 - (won_secs-WIN_MESSAGE_TIME+WIN_MESSAGE_FADE)/WIN_MESSAGE_FADE if won_secs > WIN_MESSAGE_TIME - WIN_MESSAGE_FADE else 1.0
				glColor4f(0.0, 0.0, 0.5, alpha)
				glRasterPos2f(app.winsize[0]/2 - 50, app.winsize[1]/2)
				for c in msg:
					glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, ord(c))


class MinDistanceFunction:
	"""A function-like class that returns True when the given GameObj is more than some minimum distance from a given position."""
	def __init__(self, gameobj, pos, delta):
		self.gameobj = gameobj
		self.pos = pos
		self.delta = delta
	
	def __call__(self):
		if self.gameobj.pos.dist_to(self.pos) > self.delta:
			return True


class AllRingsPassedFunction:
	"""A function-like class that returns True when all the target.Ring objects have been passed through."""
	def __call__(self):
		for obj in app.objects:
			if isinstance(obj, target.Ring):
				if not obj.passedThru:
					return False
		return True
