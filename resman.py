import pygame, os, StringIO

import app
from geometry import *
from gl import *

class SoundClip(object):
	"""A sound clip.

	Data attributes:
	filename -- The filename that the sound was loaded from.
	snd -- The pygame.mixer.Sound object.
	"""
	
	_cache = {} #Key: filename, value: SoundClip instance
	
	def __new__(cls, filename):
		"""Creates a SoundClip from an audio file, using pre-cached version if it exists."""
		
		if SoundClip._cache.has_key(filename):
			return SoundClip._cache[filename]
		else:
			obj = object.__new__(cls)
			obj.filename = filename
			SoundClip._cache[filename] = obj
			#fullpath = os.path.join(app.APP_DIR, 'sounds', filename) #FIXME: Turn this on when we eventually have some actual Orbit Ribbon sounds
			fullpath = filename
			obj.snd = pygame.mixer.Sound(fullpath)
			return obj


class Texture(object):
	"""An OpenGL 2D texture.
	
	Data attributes:
	filename -- The filename that the texture was loaded from.
	glname -- The OpenGL texture name.
	size -- The dimensions of the texture.
	surf -- The PyGame surface.
	"""
	
	_cache = {} #Key: filename, value: Texture instance
	
	def __new__(cls, filename, datafunc = None):
		"""Creates a Texture from an image file, using pre-cached version if it exists.
		
		If datafunc is specified, calls that to get the contents of the file instead of trying to open filename directly.
		"""
		
		if Texture._cache.has_key(filename):
			return Texture._cache[filename]
		else:
			obj = object.__new__(cls)
			obj.filename = filename
			Texture._cache[filename] = obj
			obj.glname = glGenTextures(1)
			if not datafunc:
				fh = file(os.path.join(app.APP_DIR, 'images', filename))
			else:
				fh = StringIO.StringIO(datafunc())
			surf = pygame.image.load(fh)
			fh.close()
			obj.surf = surf
			obj.size = (surf.get_width(), surf.get_height())
			texData = pygame.image.tostring(surf, "RGBA", 1)
			glBindTexture(GL_TEXTURE_2D, obj.glname)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf.get_width(), surf.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texData)
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
			return obj
	
	def draw_2d(self, left, top, width, height):
		"""Draws the texture as a 2D image at the given position and size.
		
		OpenGL must be in a proper state for drawing 2D images before you call this.
		"""
		glEnable(GL_TEXTURE_2D)
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE)
		glBindTexture(GL_TEXTURE_2D, self.glname)
		glBegin(GL_QUADS)
		glTexCoord2f(0.0, 1.0)
		glVertex2f(left, top)
		glTexCoord2f(1.0, 1.0)
		glVertex2f(left + width, top)
		glTexCoord2f(1.0, 0.0)
		glVertex2f(left + width, top + height)
		glTexCoord2f(0.0, 0.0)
		glVertex2f(left, top + height)
		glEnd()
		glDisable(GL_TEXTURE_2D)

def unload_all():
	"""Unloads all resources.
	
	Invalidates all instances of any of the classes in this module."""
	if len(Texture._cache) > 0:
		glDeleteTextures([ x.glname for x in Texture._cache.values() ])
		Texture._cache = {}
	
	for x in SoundClip._cache.values():
		x.snd = None # Explicitly invalidate the pygame Sound objects in case any SoundClip instances lying around
	SoundClip._cache = {}
