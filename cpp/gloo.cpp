/*
gloo.cpp: Implementation of GL Object-Oriented utility classes.

Copyright 2009 David Simon. You can reach me at david.mike.simon@gmail.com

This file is part of Orbit Ribbon.

Orbit Ribbon is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Orbit Ribbon is distributed in the hope that it will be awesome,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Orbit Ribbon.  If not, see http://www.gnu.org/licenses/
*/

#include <string>
#include <sstream>
#include <GL/glew.h>
#include <GL/gl.h>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "cache.h"
#include "debug.h"
#include "except.h"
#include "gloo.h"
#include "resman.h"

GLOOPushedMatrix::GLOOPushedMatrix() {
	glPushMatrix();
}

GLOOPushedMatrix::~GLOOPushedMatrix() {
	glPopMatrix();
}

struct SDLSurf {
	SDL_Surface* s;	
	SDLSurf(SDL_Surface* i) : s(i) { if (!i) throw GameException(std::string("Error while loading texture surface: ") + SDL_GetError()); }
	~SDLSurf() { SDL_FreeSurface(s); }
};

boost::shared_ptr<GLOOTexture> _TextureCache::generate(const std::string& id) {
	try {
		boost::shared_ptr<GLOOTexture> tex(new GLOOTexture());
		
		// Create an SDL surface from the requested ORE image file
		boost::shared_ptr<OreFileHandle> fh = ResMan::pkg().get_fh(std::string("image-") + id);
		SDL_RWops sdl_rwops = fh->get_sdl_rwops();
		SDL_Surface* surf = IMG_Load_RW(&sdl_rwops, 0);
		if (!surf) {
			throw OreException("Unable to create SDL surface");
		}
		if (
			surf->format->Rshift != 0 ||
			surf->format->Gshift != 8 ||
			surf->format->Bshift != 16 ||
			(surf->format->BitsPerPixel != 24 && surf->format->BitsPerPixel != 32)
		) {
			Debug::error_msg("BPP " + boost::lexical_cast<std::string>((unsigned int)surf->format->BitsPerPixel));
			Debug::error_msg("R " + boost::lexical_cast<std::string>((unsigned int)surf->format->Rshift));
			Debug::error_msg("G " + boost::lexical_cast<std::string>((unsigned int)surf->format->Gshift));
			Debug::error_msg("B " + boost::lexical_cast<std::string>((unsigned int)surf->format->Bshift));
			Debug::error_msg("A " + boost::lexical_cast<std::string>((unsigned int)surf->format->Ashift));
			throw OreException("Unknown pixel format");
		}
		GLenum img_format = (surf->format->BitsPerPixel == 24 ? GL_RGB : GL_RGBA);
		
		// Load the texture with the image
		tex->_width = surf->w;
		tex->_height = surf->h;
		glGenTextures(1, &(tex->_tex_name));
		glBindTexture(GL_TEXTURE_2D, tex->_tex_name);
		SDL_LockSurface(surf);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA, surf->w, surf->h, 0, img_format, GL_UNSIGNED_BYTE, surf->pixels);
		SDL_UnlockSurface(surf);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		
		SDL_FreeSurface(surf);
		
		if (glGetError()) {
			throw GameException("GL error while loading texture");
		}
		
		return tex;
	} catch (const std::exception& e) {
		throw GameException("Unable to load texture " + id + " : " + e.what());
	}
}

_TextureCache cache;

boost::shared_ptr<GLOOTexture> GLOOTexture::create(const std::string& name) {
	return cache.get(name);
}

GLOOTexture::~GLOOTexture() {
	glDeleteTextures(1, &_tex_name);
}

GLOOVertexBuffer::GLOOVertexBuffer(GLfloat* data, GLuint elements) {
	glGenBuffers(1, &_buf_name);
}

boost::shared_ptr<GLOOVertexBuffer> GLOOVertexBuffer::create(GLfloat* data, GLuint elements) {
	return boost::shared_ptr<GLOOVertexBuffer>(new GLOOVertexBuffer(data, elements));
}

GLOOVertexBuffer::~GLOOVertexBuffer() {
	glDeleteBuffers(1, &_buf_name);
}
