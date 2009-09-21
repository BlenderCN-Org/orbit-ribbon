#!/usr/bin/python

import shuttle
from shuttle.cmds import *

shuttle.init()

glShadeModel

glPushMatrix()
glColor3f(1.0, 0.0, 0.0)
glVertex3f(-1.0, 1.0, 1.0)
glPopMatrix()

shuttle.flush()
