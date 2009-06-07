import ode
from OpenGL.GL import *
from OpenGL.GLUT import *

import app, gameobj, colors, collision, resman
from geometry import *
from util import *

class Cube(gameobj.GameObj):
	"""A solid-colored cube for testing.
	
	Data attributes:
	color - The color of the cube (defaults to red).
	"""
	
	def __init__(self, pos, color = colors.red, scale = 1.0):
		geom = ode.GeomBox(app.dyn_space, (scale, scale, scale))
		geom.coll_props = collision.Props()
		super(Cube, self).__init__(pos = pos, body = sphere_body(1, 0.375), geom = geom)
		self.color = color
		self._scale = scale
	
	def indraw(self):
		glColor3fv(self.color)
		glutSolidCube(self._scale)


class Ground(gameobj.GameObj):
	"""A non-movable planar ground object for testing."""

	def __init__(self, pos, scale = 100.0):
		geom = ode.GeomBox(app.static_space, (scale, 0.01, scale))
		geom.coll_props = collision.Props()
		super(Ground, self).__init__(pos = pos, body = None, geom = geom)
		self._scale = scale
		self._tex = resman.Texture("diffract.png")
	
	def indraw(self):
		scale = self._scale
		tscale = 4 # Scaling of the texture
		glEnable(GL_TEXTURE_2D)
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE)
		glBindTexture(GL_TEXTURE_2D, self._tex.glname)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
		glBegin(GL_QUADS)
		glTexCoord2f(0.0, 0.0)
		glVertex3f(-scale/2, 0, -scale/2)
		glTexCoord2f(0.0, scale/tscale)
		glVertex3f(-scale/2, 0, scale/2)
		glTexCoord2f(scale/tscale, scale/tscale)
		glVertex3f(scale/2, 0, scale/2)
		glTexCoord2f(scale/tscale, 0.0)
		glVertex3f(scale/2, 0, -scale/2)
		glEnd()
		glDisable(GL_TEXTURE_2D)


class GreenSphere(gameobj.GameObj):
	"""A jungle-like sphere for testing."""
	
	def __init__(self, pos, radius = 10000):
		geom = ode.GeomSphere(app.static_space, radius)
		geom.coll_props = collision.Props()
		super(GreenSphere, self).__init__(pos = pos, body = None, geom = geom)
		self._radius = radius
	
	def indraw(self):
		glColor3f(0.0, 0.7, 0.0)
		glutSolidSphere(self._radius, 20, 20)
