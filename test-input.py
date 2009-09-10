#!/usr/bin/python

import time, pygame

import input

pygame.display.init()
pygame.display.set_caption('Input Test')
screen = pygame.display.set_mode((100, 100))

m = input.InputManager()

while True:
	time.sleep(0.01)
	m.update()
	
	on_list = []
	for c in m.all_channels:
		if c.is_on():
			on_list.append("%s:%.03f" % (c.desc(), c.value()))
	if len(on_list) > 0:
		print ", ".join(on_list)
