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

_BufferAllocManager::AllocToken _BufferAllocManager::allocate(unsigned int length) {
	Allocation alloc;
	alloc.length = length;
	alloc.offset = 0;
	if (!_allocs.empty()) {
		std::list<Allocation>::iterator end = _allocs.end();
		alloc.offset = end->offset + end->length;
	}
	if (alloc.offset + alloc.length > _max_length) {
		throw OreException("Ran out of space while attempting GL buffer allocation!");
	}
	_allocs.push_back(alloc);
	return _allocs.end()-1;
}

_MeshBufferManager::MeshBOInfo::MeshBOInfo(BufferAllocManager::Allocation vbo_alloc, BufferAllocManager::Allocation ibo_alloc) :
	_vbo_alloc(vbo_alloc),
	_ibo_alloc(ibo_alloc),
	_vertices_added(0),
	_faces_added(0)
{
	_vbo_mapping = (GLOOVertex*)glMapBufferRange(GL_ARRAY_BUFFER, _vbo_alloc.get_offset(), _vbo_alloc.get_length(), GL_MAP_WRITE_BIT);
	_ibo_mapping = (GLOOFace*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, _ibo_alloc.get_offset()*3, _ibo_alloc.get_length()*3, GL_MAP_WRITE_BIT);
}

void _MeshBufferManager::MeshBOInfo::add_vertex(const GLOOVertex& v) {
	if (_vertices_added >= _vbo_alloc.get_length()) {
		throw OreException("Attempted to add more vertices than space was allocated for")
	}
	*_vbo_mapping = v;
	++_vbo_mapping;
	++_vertices_added;
}

void _MeshBufferManager::MeshBOInfo::add_face(const GLOOFace& f) {
	if (_faces_added >= _ibo_alloc.get_length()) {
		throw OreException("Attempted to add more faces than space was allocated for")
	}
	*_ibo_mapping = f;
	++_ibo_mapping;
	++_faces_added;
}

_MeshBufferManager::_MeshBufferManager() :
	_vbo_aman((VBO_ALLOCATED_SIZE*1024)/sizeof(GLOOVertex)),
	_ibo_aman((IBO_ALLOCATED_SIZE*1024)/sizeof(GLOOFace)),
	anything_mapped(false)
{
	glGenBuffers(1, &_vboid);
	glBindBuffer(GL_ARRAY_BUFFER, _vboid);
	glBufferData(GL_ARRAY_BUFFER, VBO_ALLOCATED_SIZE*1024, NULL, GL_STATIC_DRAW);
	
	glGenBuffers(1, &_iboid);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _iboid);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IBO_ALLOCATED_SIZE*1024, NULL, GL_STATIC_DRAW);
}

boost::shared_ptr<_MeshBufferManager::MeshBOInfo> _MeshBufferManager::allocate(unsigned int vertexCount, unsigned int faceCount) {
	if (anything_mapped) {
		throw OreException("Attempted to allocate from VBO/IBO while a mapping was already active!");
	}
	
	anything_mapped = true;
	return boost::shared_ptr<_MeshBufferManager::MeshBOInfo>(new _MeshBuffManager::MeshBOInfo(vertexCount, faceCount));
}

void _MeshBufferManager::unmap(const _MeshBufferManager::MeshBOInfo& bo_info) {
	if (anything_mapped && bo_info.mapped) {
		bo_info._vbo_mapping = 0;
		bo_info._ibo_mapping = 0;
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		anything_mapped = false;
	}
}

void _MeshBufferManager::free(const _MeshBufferManager::MeshBOInfo& bo_info) {
	unmap(bo_info);
	_vbo_aman.free(bo_info._vbo_alloc);
	_ibo_aman.free(bo_info._ibo_alloc);
}

_MeshBufferManager::~_MeshBufferManager() {
	glDeleteBuffers(1, &_vboid);
	glDeleteBuffers(1, &_iboid);
}

GLOOBufferedMesh::GLOOBufferedMesh(unsigned int vertexCount, unsigned int faceCount) {
	static MeshBufferManager buf_man;
	_bo_info = buf_man.allocate(vertexCount, faceCount);
}

boost::shared_ptr<GLOOBufferedMesh> GLOOBufferedMesh::create(unsigned int vertexCount, unsigned int faceCount) {
	return boost::shared_ptr<GLOOBufferedMesh>(new GLOOBufferedMesh(vertexCount, faceCount));
}

void GLOOBufferedMesh::load_vertex(const GLOOVertex& v) {
	_bo_info->add_vertex(v);
}

void GLOOBufferedMesh::load_face(const GLOOFace& v) {
	_bo_info->add_face(f);
}

void GLOOBufferedMesh::finish_loading() {
	// FIXME AAAARRGH! Need to rethink this; avoid use of static locals, and reduce number of classes
}

void GLOOBufferedMesh::draw() {
}

GLOOBufferedMesh::~GLOOBufferedMesh() {
}