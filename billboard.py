from __future__ import division

import math
import OpenGL
OpenGL.ERROR_CHECKING = False
OpenGL.ERROR_LOGGING = False
from OpenGL.GL import *

from util import *

def draw_billboard(pos, tex, width, camVec):
	"""Draws a billboard, a texture which faces the camera.
	
	Be sure to enable GL_TEXTURE_2D and set the texture environment mode to GL_REPLACE before calling this.
	"""
	# If it's too far away compared to its size, don't bother with it
	# FIXME Come up with something meaningful here, not just this fudge value of 15
	if width*width/camVec.mag() < 15:
		return
	
	glBindTexture(GL_TEXTURE_2D, tex.glname)
	glPushMatrix()
	glTranslatef(*pos)
	
	# Rotate the billboard so that it faces the camera
	glRotatef(rev2deg(rad2rev(math.atan2(camVec[0], camVec[2]))), 0, 1, 0) # Rotate around y-axis...
	glRotatef(-rev2deg(rad2rev(math.atan2(camVec[1], math.sqrt(camVec[0]**2 + camVec[2]**2)))), 1, 0, 0) # Then tilt up/down on x-axis
	glBegin(GL_QUADS)
	glTexCoord2f(0.0, 0.0)
	glVertex3f(-width/2, -width/2, 0)
	glTexCoord2f(1.0, 0.0)
	glVertex3f( width/2, -width/2, 0)
	glTexCoord2f(1.0, 1.0)
	glVertex3f( width/2,  width/2, 0)
	glTexCoord2f(0.0, 1.0)
	glVertex3f(-width/2,  width/2, 0)
	glEnd()
	glPopMatrix()
