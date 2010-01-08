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

#include <memory>
#include <list>
#include <string>
#include <sstream>
#include <GL/glew.h>
#include <GL/gl.h>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "cache.h"
#include "constants.h"
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
	SDLSurf(SDL_Surface* i) : s(i) {
		if (!i) throw GameException(std::string("Error while loading texture surface: ") + SDL_GetError());
		SDL_LockSurface(s);
	}
	~SDLSurf() { SDL_UnlockSurface(s); SDL_FreeSurface(s); }
	SDL_Surface* operator->() { return s; }
	SDL_Surface& operator*() { return *s; }
};

boost::shared_ptr<GLOOTexture> _TextureCache::generate(const std::string& id) {
	try {
		boost::shared_ptr<GLOOTexture> tex(new GLOOTexture());
		
		// Create an SDL surface from the requested ORE image file
		boost::shared_ptr<OreFileHandle> fh = ResMan::pkg().get_fh(std::string("image-") + id);
		SDL_RWops sdl_rwops = fh->get_sdl_rwops();
		SDLSurf surf(IMG_Load_RW(&sdl_rwops, 0)); //SDLSurf takes care of locking surface now and later unlocking/freeing it
		if (
			surf->format->Rshift != 0 ||
			surf->format->Gshift != 8 ||
			surf->format->Bshift != 16 ||
			(surf->format->BitsPerPixel != 24 && surf->format->BitsPerPixel != 32)
		) {
			throw OreException(
				std::string("Unknown pixel format :") +
				" BPP " + boost::lexical_cast<std::string>((unsigned int)surf->format->BitsPerPixel) +
				" Rs " + boost::lexical_cast<std::string>((unsigned int)surf->format->Rshift) + 
				" Gs " + boost::lexical_cast<std::string>((unsigned int)surf->format->Gshift) +
				" Bs " + boost::lexical_cast<std::string>((unsigned int)surf->format->Bshift) +
				" As" + boost::lexical_cast<std::string>((unsigned int)surf->format->Ashift)
			);
		}
		GLenum img_format = (surf->format->BitsPerPixel == 24 ? GL_RGB : GL_RGBA);
		
		// Load the texture with the image
		tex->_width = surf->w;
		tex->_height = surf->h;
		glGenTextures(1, &(tex->_tex_name));
		glBindTexture(GL_TEXTURE_2D, tex->_tex_name);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA, surf->w, surf->h, 0, img_format, GL_UNSIGNED_BYTE, surf->pixels);
		glEnable(GL_TEXTURE_2D); // Work around a bug on some ATI drivers
		glGenerateMipmap(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);
		
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

_VBOManager::Allocation::Allocation(unsigned int bytes, _VBOManager* man) :
	_man(man)
{
	AllocRange rng;
	rng.bytes = bytes;
	rng.offset = 0;
	if (!_man->_ranges.empty()) {
		rng.offset = _man->_ranges.back().offset + _man->_ranges.back().bytes;
	}
	if (rng.offset + rng.bytes > _man->_max_bytes) {
		throw OreException("Ran out of space while attempting allocation within VBO!");
	}
	_man->_ranges.push_back(rng);
	_iter = _man->_ranges.end();
	--_iter;
}

void* _VBOManager::Allocation::map() {
	if (!_man->_mapped) {
		throw OreException("Attempted to map memory within already-mapped VBO");
	} else {
		_man->_mapped = true;
		return glMapBufferRange(_man->_tgt, offset(), bytes(), GL_MAP_WRITE_BIT);
	}
}

void _VBOManager::Allocation::unmap() {
	if (_man->_mapped) {
		glUnmapBuffer(_man->_tgt);
		_man->_mapped = false;
	}
}

_VBOManager::Allocation::~Allocation() {
	unmap();
	_man->_ranges.erase(_iter);
}

_VBOManager::_VBOManager(GLenum tgt, unsigned int max_bytes) :
	_tgt(tgt),
	_max_bytes(max_bytes),
	_mapped(false)
{
	glGenBuffers(1, &_buf_id);
	glBindBuffer(tgt, _buf_id);
	glBufferData(tgt, max_bytes, NULL, GL_STATIC_DRAW);
}

_VBOManager::~_VBOManager() {
	glDeleteBuffers(1, &_buf_id);
}

bool GLOOBufferedMesh::_initialized = false;
boost::scoped_ptr<_VBOManager> GLOOBufferedMesh::_vertices_vboman, GLOOBufferedMesh::_faces_vboman;

GLOOBufferedMesh::GLOOBufferedMesh(unsigned int vertex_count, unsigned int face_count) :
	_vertices_added(0),
	_total_vertices(vertex_count),
	_faces_added(0),
	_total_faces(face_count)
{
	if (!_initialized) {
		_vertices_vboman.reset(new _VBOManager(GL_ARRAY_BUFFER, VERTICES_BUFFER_ALLOCATED_SIZE*1024));
		_faces_vboman.reset(new _VBOManager(GL_ELEMENT_ARRAY_BUFFER, FACES_BUFFER_ALLOCATED_SIZE*1024));
		_initialized = true;
	}
	
	_vertices_alloc = _vertices_vboman->allocate(_total_vertices*sizeof(GLOOVertex));
	_next_vertex = (GLOOVertex*)_vertices_alloc->map();
	
	_faces_alloc = _faces_vboman->allocate(_total_faces*sizeof(GLOOFace));
	_next_face = (GLOOFace*)_faces_alloc->map();
}

void GLOOBufferedMesh::load_vertex(const GLOOVertex& v) {
	if (_vertices_added >= _total_vertices) {
		throw OreException("Attempted to load more vertices than space was allocated for");
	}
	if (!_next_vertex) {
		throw OreException("Attempted to load a vertex while unmapped");
	}
	*_next_vertex = v;
	++_next_vertex;
	++_vertices_added;
}

void GLOOBufferedMesh::load_face(const GLOOFace& f) {
	if (_faces_added >= _total_faces) {
		throw OreException("Attempted to load more faces than space was allocated for");
	}
	if (!_next_face) {
		throw OreException("Attempted to load a face while unmapped");
	}
	*_next_face = f;
	++_next_face;
	++_faces_added;
}

void GLOOBufferedMesh::finish_loading() {
	if (_next_vertex == 0 && _next_face == 0) {
		return;
	}
	
	if (_vertices_added != _total_vertices) {
		throw OreException((boost::format("Allocated space for %1% vertices, but only loaded %2%") % _total_vertices % _vertices_added).str());
	}
	
	if (_faces_added != _total_faces) {
		throw OreException((boost::format("Allocated space for %1% faces, but only loaded %2%") % _total_faces % _faces_added).str());
	}
	
	_vertices_alloc->unmap();
	_next_vertex = 0;
	_faces_alloc->unmap();
	_next_face = 0;
}

void GLOOBufferedMesh::draw() {
	if (_vertices_vboman->mapped() or _faces_vboman->mapped()) {
		throw OreException("Attempted to draw GLOOBufferedMesh while still mapped");
	}
}

GLOOBufferedMesh::~GLOOBufferedMesh() {
	finish_loading();
}