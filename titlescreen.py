from __future__ import division

from OpenGL.GL import *

import app, resman

TSMODE_MAIN, TSMODE_AREA, TSMODE_MISSION = range(3)

class TitleScreenManager:
	"""Manages the behavior of the pre-gameplay virtual interface."""
	
	def __init__(self):
		self._title_tex = resman.Texture("title.png")
		self._tsmode = TSMODE_MAIN
	
	def draw(self):
		if self._tsmode == TSMODE_MAIN:
			### Draw the main title screen interface

			# Draw the title logo
			top_margin = app.winsize[1]/60
			left_margin = app.winsize[0]/40
			right_margin = app.winsize[0]/6
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
			
		elif self._tsmode == TSMODE_AREA:
			### Draw the area selection interface
			pass
		elif self._tsmode == TSMODE_MISSION:
			### Draw the mission selection interface
			pass
