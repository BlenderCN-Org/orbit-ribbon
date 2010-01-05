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
	SDLSurf(SDL_Surface* i) : s(i) { if (!i) throw GameException("Error while loading texture surface"); }
	~SDLSurf() { SDL_FreeSurface(s); }
};

boost::shared_ptr<GLOOTexture> _TextureCache::generate(const std::string& id) {
	try {
		boost::shared_ptr<GLOOTexture> tex(new GLOOTexture());
		
		boost::shared_ptr<OreFileHandle> fh = ResMan::pkg().get_fh(std::string("image-") + id);
		std::stringstream raw;
		(*fh) >> raw.rdbuf();
		SDL_RWops* sdl_raw = SDL_RWFromConstMem(raw.str().data(), raw.str().size());
		SDLSurf surf(IMG_Load_RW(sdl_raw, 0));
		tex->width = surf.s->w;
		tex->height = surf.s->h;
		Debug::debug_msg("BYTES PER PIXEL: " + boost::lexical_cast<std::string>(surf.s->format->BytesPerPixel));
		
		try {
			glGenTextures(1, &(tex->_tex_name));
			
		} catch (const std::exception& e) {
			glDeleteTextures(1, &(tex->_tex_name));
			throw e;
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
