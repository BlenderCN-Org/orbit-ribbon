#!/usr/bin/python

import time, pygame
from pygame.locals import *

import inputs

pygame.display.init()
pygame.display.set_caption('Input Test')
screen = pygame.display.set_mode((400, 550), DOUBLEBUF)

pygame.font.init()
font = pygame.font.Font(None, 24)

m = inputs.InputManager()

class QuitException(Exception):
	pass

while True:
	time.sleep(0.05)
	m.update()
	
	try:
		for event in pygame.event.get():
			if event.type == pygame.QUIT:
				raise QuitException
	except QuitException:
		break

	screen.fill((0,0,0))
	
	y = 10
	for c in m.all_channels:
		if c.is_on():
			s = "%s:%.03f" % (c.desc(), c.value())
			print s
			surf = font.render(s, True, (255,255,255), (0,0,0))
			screen.blit(surf, (5, y))
			y += font.get_linesize()
	
	pygame.display.flip()
