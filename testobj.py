import ode
from OpenGL.GL import *
from OpenGL.GLUT import *

import app, gameobj, colors, collision, resman, billboard
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
	"""A non-movable rectangular ground object for testing."""
	
	def __init__(self, pos, xscale = 100.0, zscale = 100.0):
		geom = ode.GeomBox(app.static_space, (xscale, 1.0, zscale))
		geom.coll_props = collision.Props()
		super(Ground, self).__init__(pos = pos, body = None, geom = geom)
		self._xscale = xscale
		self._zscale = zscale
		self._tex = resman.Texture("diffract.png")
	
	def indraw(self):
		xscale = self._xscale
		zscale = self._zscale
		tscale = 0.25 # Scaling of the texture
		glEnable(GL_TEXTURE_2D)
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE)
		glBindTexture(GL_TEXTURE_2D, self._tex.glname)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
		y = 0.5
		tul = Point(-xscale/2,  y, -zscale/2)
		tll = Point(-xscale/2,  y,  zscale/2)
		tlr = Point( xscale/2,  y,  zscale/2)
		tur = Point( xscale/2,  y, -zscale/2)
		bul = Point(-xscale/2, -y, -zscale/2)
		bll = Point(-xscale/2, -y,  zscale/2)
		blr = Point( xscale/2, -y,  zscale/2)
		bur = Point( xscale/2, -y, -zscale/2)
		glBegin(GL_QUADS)
		for a, b, c, d in (
			(tul, tll, tlr, tur),
			(bul, bll, blr, bur),
			(tul, bul, bur, tur),
			(tll, bll, blr, tlr),
			(tul, bul, bll, tll),
			(tur, bur, blr, tlr),
		):
			ab_dist = a.dist_to(b)
			bc_dist = b.dist_to(c)
			glTexCoord2f(0.0, 0.0)
			glVertex3f(*a)
			glTexCoord2f(0.0, ab_dist*tscale)
			glVertex3f(*b)
			glTexCoord2f(bc_dist*tscale, ab_dist*tscale)
			glVertex3f(*c)
			glTexCoord2f(bc_dist*tscale, 0.0)
			glVertex3f(*d)
		glEnd()
		glDisable(GL_TEXTURE_2D)


class GreenSphere(gameobj.GameObj):
	"""A jungle-like sphere for testing."""
	
	def __init__(self, pos, radius = 10000):
		geom = ode.GeomSphere(app.static_space, radius)
		geom.coll_props = collision.Props()
		super(GreenSphere, self).__init__(pos = pos, body = None, geom = geom)
		self._radius = radius
		self._jungle_tex = resman.Texture("jungle.png")
	
	def indraw(self):
		glColor3f(0.0, 0.7, 0.0)
		glutSolidSphere(self._radius, 20, 20)
	
	def indistdraw(self):
		billboard.draw_billboard(self.pos, self._jungle_tex, self._radius*2, -self.pos + app.player_camera.get_position())
