#This is where the debugging console runs
from __future__ import division

import sys
import math
import os
import re

import ode
import pygame
import pydoc
from pygame.locals import *

import app
import avatar
import camera
import collision
import colors
import console
import gameobj
from geometry import *
import joy
import mission
import resman
import sky
import target
import testobj
from util import *

def sc_camy():
	wmod(2, "avatar.CAMERA_OFFSET_Y", 1000000)
	wset(3, "app.player_camera")

def sc_gang():
	wmod(2, "app.sky_stuff.game_angle", 0.05)
	wset(3, "app.sky_stuff._localGamePos")

def wset(num, expr):
	"""Shortcut for 'app.watchers[num].expr = expr', and clears modVal."""
	app.watchers[num].expr = expr
	app.watchers[num].modVal = None

def wmod(num, expr, v):
	"""Shortcut for 'app.watchers[num].expr = expr; app.watchers[num].modVal = v'."""
	app.watchers[num].expr = expr
	app.watchers[num].modVal = v
	
def wclear(num = -1):
	"""Clears all Watchers in app.watchers, or just the specified one."""
	if num == -1:
		for w in app.watchers:
			w.expr = None
			w.modVal = None
	else:
		app.watchers[num].expr = None
		app.watchers[num].modVal = None

def wfps(num = 0):
	"""Sets a given watcher (#0 by default) to show the FPS."""
	app.watchers[num].expr = "'%.2f' % app.clock.get_fps()"

def objs():
	"""Short for 'print app.objects'"""
	print app.objects

class ConsDoc(pydoc.TextDoc):
	#The regular bolder tries to replace X with X<BKSP>X
	#That doesn't work very well for the debugging console output
	def bold(self, text):
		return text

def help(tgt = None):
	if tgt == None:
		print console.helphelp
	else:
		print ConsDoc().document(tgt).strip()

def quit():
	print console.quithelp

def exit():
	print console.quithelp

def close():
	print console.quithelp
