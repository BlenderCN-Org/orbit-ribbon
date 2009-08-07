from __future__ import division

import pygame

import ore, app

class AnimManager:
	"""A class that moves an object through the frames of an OREAnimation.
	
	Note that the timing is based upon the step count in app, not the wall clock time. Therefore, AnimManager
	can only be used during gameplay when the simulation clock is running.
	
	Data attributes:
	ore_anim - The ore.OREAnimation to be used.
	cur_frame_idx - The index of the last frame that was drawn. None if the AnimManager has been reset or is new.
	cur_frame_steps - The step count at which the last frame was drawn. None if the AnimManager has been reset or is new.
	reverse - If True, the animation moves from the last frame to the first frame.
	repeat - If True, the animation starts over again as soon as it reaches the end.
	lock - If True, the animation only shows the first frame (or, if reverse is also True, the last frame).
	"""
	
	def __init__(self, ore_anim, reverse = False, repeat = False, lock = False):
		self.ore_anim = ore_anim
		self.cur_frame_idx = None
		self.cur_frame_steps = None
		self.reverse = reverse
		self.repeat = repeat
		self.lock = lock
	
	def reset(self):
		"""Resets the animation back to the first frame."""
		self.cur_frame_idx = None
		self.cur_frame_steps = None
	
	def on_last_frame(self):
		"""Returns True if the previous frame returned by cur_frame() was the last frame in the animation."""
		tgt_frame = len(self.ore_anim.frames)-1 if not self.reverse else 0
		if self.cur_frame_idx == tgt_frame:
			return True
		return False
	
	def cur_frame(self):
		"""Returns the OREMesh for the current frame, and advances the frame timer.
		
		After a new AnimManager is created, or after it is reset, the timer is frozen until this is first called.
		"""
		if self.cur_frame_idx is None:
			if self.reverse:
				self.cur_frame_idx = len(self.ore_anim.frames)-1
			else:
				self.cur_frame_idx = 0
		elif not self.lock:
			diff = app.totalsteps - self.cur_frame_steps
			if self.reverse:
				diff = -diff
			self.cur_frame_idx += diff
			if self.repeat:
				self.cur_frame_idx = self.cur_frame_idx % len(self.ore_anim.frames)
			else:
				self.cur_frame_idx = min(self.cur_frame_idx, len(self.ore_anim.frames)-1)
		
		self.cur_frame_steps = app.totalsteps
		return self.ore_anim.frames[self.cur_frame_idx]
	
	def cur_frame_static(self):
		"""Returns the OREMesh for the current frame, but does not advance the timer.
		
		Do not call this method unless you have previously called cur_frame. Another way to describe this method
		is that it returns whatever cur_frame() returned the last time you called it.
		
		This is useful for calling from indraw() on GameObjs which advance the frame on step(). That way, you can
		use the animation for game logic and also for drawing.
		"""
		return self.ore_anim.frames[self.cur_frame_idx]
