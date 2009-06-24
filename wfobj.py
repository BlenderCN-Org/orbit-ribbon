from __future__ import division

import ode
from OpenGL.GL import *

import app, gameobj
from geometry import *
from util import *

class WFObj(gameobj.GameObj):
	"""A GameObj that loads its data from a WaveFront model file."""

	def __init__(self, fn, pos = None, rot = None, body = None):
		"""Creates a WFObj from the given filename. Other arguments passed to GameObj init."""
		fh = file(fn)
		self._verts = []
		self._vnorms = []
		self._faces = []
		for line in fh:
			args = line.split()
			cmd = args.pop(0)
			if cmd == "v":
				self._verts.append(tuple([float(x) for x in args]))
			elif cmd == "vn":
				self._vnorms.append(tuple([float(x) for x in args]))
			elif cmd == "f":
				fvals = [] # Alternating between normal and vertex for all points on this face
				for x in args:
					vi, ni = x.split("//")
					fvals.append(int(ni)-1)
					fvals.append(int(vi)-1)
				self._faces.append(tuple(fvals))
		
		# FIXME Temporary testing value for geom
		super(WFObj, self).__init__(pos = pos, rot = rot, body = body, geom = ode.GeomBox(app.static_space, (0.1, 0.1, 0.1)))
		
		# FIXME Should free list on destruction
		self._glListNum = glGenLists(1)
		glNewList(self._glListNum, GL_COMPILE)
		glColor3f(0.0, 0.6, 0.0)
		glBegin(GL_TRIANGLES)
		for f in self._faces:
			glNormal3f(*self._vnorms[f[0]])
			glVertex3f(*self._verts[f[1]])
			glNormal3f(*self._vnorms[f[2]])
			glVertex3f(*self._verts[f[3]])
			glNormal3f(*self._vnorms[f[4]])
			glVertex3f(*self._verts[f[5]])
		glEnd()
		glEndList()
	
	def indraw(self):
		glCallList(self._glListNum)
