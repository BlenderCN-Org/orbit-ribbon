#!/usr/bin/python

import time

import shuttle
from shuttle.cmds import *

shuttle.init()

glShadeModel(GL_SMOOTH)
glClearColor(0, 0, 0, 0)
glClearDepth(1)
glEnable(GL_DEPTH_TEST)
glDepthFunc(GL_LEQUAL)
glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST)

glViewport(0, 0, 800, 600)
glMatrixMode(GL_PROJECTION)
glLoadIdentity()
gluPerspective(45, 800/600, 0.1, 100)
glMatrixMode(GL_MODELVIEW)
glLoadIdentity()

rtri, rquad = 0, 0
start_time = time.time()
cur_time = start_time
frames = 0

while True:
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
	
	glPushMatrix()
	glTranslatef(0, 0, -6)
	glScalef(0.05, 0.05, 0.05)

	for row in xrange(30):
		for col in xrange(40):
			glPushMatrix()
			glTranslatef(-60 + col*3, 45 - row*3, 0)
			
			glPushMatrix()
			glRotatef(rtri, 0, 1, 0)
			glBegin(GL_TRIANGLES)
			glColor3f(1, 0, 0)
			glVertex3f(0, 1, 0)
			glColor3f(0, 1, 0)
			glVertex3f(-1, -1, 0)
			glColor3f(0, 0, 1)
			glVertex3f(1, -1, 0)
			glEnd()
			glPopMatrix()

			glPushMatrix()
			glTranslatef(0, 0, -3)
			glRotatef(rquad, 1, 0, 0)
			glColor3f(0.5, 0.5, 1)
			glBegin(GL_QUADS)
			glVertex3f(1, 1, 0)
			glVertex3f(-1, 1, 0)
			glVertex3f(-1, -1, 0)
			glVertex3f(1, -1, 0)
			glEnd()
			glPopMatrix()
			
			glPopMatrix()
	
	glPopMatrix()
	
	shuttle.flush()
	
	frames += 1
	
	new_time = time.time()
	rtri += 0.2*(new_time - cur_time)*1000
	rquad -= 0.15*(new_time - cur_time)*1000
	cur_time = new_time

	if cur_time - start_time > 10:
		secs = cur_time - start_time
		fps = frames/secs
		print "%u frames in %.4f seconds = %.4f FPS" % (frames, secs, fps)
		break

shuttle.quit()
