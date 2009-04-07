#!/usr/bin/python

print "Loading..."

import pygame
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

pygame.init()
pygame.joystick.init()

screen = pygame.display.set_mode((640,480), OPENGL|DOUBLEBUF)

clock = pygame.time.Clock()

js = pygame.joystick.Joystick(0)
js.init()

class QuitException(Exception):
	pass

BUTTON_NUMS = {
	"SELECT" : 0,
	"START" : 3,
	"L1" : 10,
#	"L2" : 8,
	"L3" : 1,
	"R1" : 11,
#	"R2" : 9,
	"R3" : 2,
	"UP" : 4,
	"LEFT" : 7,
	"RIGHT" : 5,
	"DOWN" : 6,
	"S" : 15,
	"X" : 14,
	"O" : 13,
	"T" : 12,
}
BUTTON_NAMES = dict([ (BUTTON_NUMS[n], n) for n in BUTTON_NUMS.keys() ])

AXIS_NUMS = {
	"L2" : 12,
	"LX" : 0,
	"LY" : 1,
	"R2" : 13,
	"RX" : 2,
	"RY" : 3,
}
AXIS_NAMES = dict([ (AXIS_NUMS[n], n) for n in AXIS_NUMS.keys() ])

print "Begins."

def run():
	try:
		while True:
			clock.tick(100)
			
			for e in pygame.event.get():
				if e.type == pygame.QUIT:
					raise QuitException
				elif e.type == pygame.KEYDOWN and e.key == pygame.K_q:
					raise QuitException
				elif e.type == pygame.JOYAXISMOTION:
					if e.axis in AXIS_NAMES:
						print "A %s  %f" % (AXIS_NAMES[e.axis], e.value)
				elif e.type == pygame.JOYBUTTONDOWN:
					if e.button in BUTTON_NAMES:
						print "DOWN %s" % (BUTTON_NAMES[e.button])
				elif e.type == pygame.JOYBUTTONUP:
					if e.button in BUTTON_NAMES:
						print "UP   %s" % (BUTTON_NAMES[e.button])
	except QuitException:
		pass
