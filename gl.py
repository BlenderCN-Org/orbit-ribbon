# Based on code written by Nathan Ostgard, which was retrieved from http://www.siafoo.net/snippet/185 on 2009 Aug 08

import OpenGL
OpenGL.ERROR_CHECKING = False
OpenGL.ERROR_LOGGING = False
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *
from OpenGL.raw import GL as RawGL
from OpenGL.arrays import ArrayDatatype as ADT


class GLDisplayList:
	"""A class representing an OpenGL display list.
	
	Create a DisplayList, then perform the actions to be recorded between begin_rec() and end_rec().
	Then, call play() to execute the list.
	"""
	def __init__(self):
		self._list = glGenLists(1)
	
	def __del__(self):
		glDeleteLists(self._list, 1)
	
	def begin_rec(self, mode = GL_COMPILE):
		glNewList(self._list, mode)
	
	def end_rec(self):
		glEndList()
	
	def play(self):
		glCallList(self._list)


class GLVertexBuffer:
	"""A class representing an OpenGL Vertex Buffer Object, which can hold more than just vertexes.
	
	Create a VertexBuffer using a numpy array of the data you want to load to the video hardware. At
	draw time, call the relevant bind method before calling glDrawArrays or a similar function.
	"""
	def __init__(self, data, usage):
		self._buffer = RawGL.GLuint(0)
		glGenBuffers(1, self._buffer)
		self._buffer = self._buffer.value
		self.bind()
		glBufferData(GL_ARRAY_BUFFER_ARB, ADT.arrayByteCount(data), ADT.voidDataPointer(data), usage)
	
	def __del__(self):
		glDeleteBuffers(1, RawGL.GLuint(self._buffer))
	
	def bind(self):
		glBindBuffer(GL_ARRAY_BUFFER_ARB, self._buffer)
	
	def bind_colors(self, size, type, stride=0):
		self.bind()
		glColorPointer(size, type, stride, None)
	
	def bind_edgeflags(self, stride=0):
		self.bind()
		glEdgeFlagPointer(stride, None)
	
	def bind_indexes(self, type, stride=0):
		self.bind()
		glIndexPointer(type, stride, None)
	
	def bind_interleaved(self, format, stride=0):
		self.bind()
		glInterleavedArrays(format, stride, None)
	
	def bind_normals(self, type, stride=0):
		self.bind()
		glNormalPointer(type, stride, None)
	
	def bind_texcoords(self, size, type, stride=0):
		self.bind()
		glTexCoordPointer(size, type, stride, None)
	
	def bind_vertexes(self, size, type, stride=0):
		self.bind()
		glVertexPointer(size, type, stride, None)
