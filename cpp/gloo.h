/*
gloo.h: Header for GL Object-Oriented utility classes.

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

#ifndef ORBIT_RIBBON_GLOO_H
#define ORBIT_RIBBON_GLOO_H

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <string>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include "cache.h"
#include "geometry.h"

class GLOOPushedMatrix : boost::noncopyable {
	public:
		GLOOPushedMatrix();
		~GLOOPushedMatrix();
};

class GLOOTexture;

class _TextureCache : public CacheBase<GLOOTexture> {
	boost::shared_ptr<GLOOTexture> generate(const std::string& id);
};

class GLOOTexture : boost::noncopyable {
	private:
		GLuint _tex_name;
		GLuint _width, _height;
		
		GLOOTexture() {}
		
		friend class _TextureCache;
	
	public:
		static boost::shared_ptr<GLOOTexture> create(const std::string& name);
		
		GLuint get_width() { return _width; }
		GLuint get_height() { return _height; }
		
		virtual ~GLOOTexture();
};

struct GLOOVertex {
	GLfloat x, y, z;
	GLfloat nx, ny, nz;
	GLfloat u, v;
};

struct GLOOFace {
	// Indices to the vertices forming a triangle face
	GLuint a, b, c;
};

class _BufferAllocManager : boost::noncopyable {
	private:
		struct Allocation {
			unsigned int offset, length;
		};
		
		unsigned int _max_length;
		std::list<Allocation> _allocs;
	
	public:
		_BufferAllocManager(unsigned int max_length) : _max_length(max_length) {}
		
		typedef std::list<Allocation>::iterator AllocToken;
		
		AllocToken allocate(unsigned int length);
		void free(const AllocToken& tok) { _allocs.erase(tok); }
		
		unsigned int get_offset(const AllocToken& tok) { return tok->offset; }
		unsigned int get_length(const AllocToken& tok) { return tok->length; }
};

class _MeshBufferManager;

class _MeshBufferManager : boost::noncopyable {
	private:
		GLuint _vboid, _iboid;
		BufferAllocManager _vbo_aman;
		BufferAllocManager _ibo_aman;
		bool anything_mapped;
	
	public:
		class MeshBOInfo : boost::noncopyable {
			private:
				BufferAllocManager::Allocation _vbo_alloc, _ibo_alloc;
				unsigned int _vertices_added, _faces_added;
				GLOOVertex* _vbo_mapping;
				GLOOFace* _ibo_mapping;
				
				MeshBOInfo(BufferAllocManager::Allocation vbo_alloc, BufferAllocManager::Allocation ibo_alloc);
				
				friend class _MeshBufferManager;
			
			public:
				void add_vertex(const GLOOVertex& v);
				void add_face(const GLOOFace& f);
		};
		
		_MeshBufferManager();
		
		boost::shared_ptr<MeshBOInfo> allocate(unsigned int vertexCount, unsigned int faceCount);
		void unmap(const MeshBOInfo& bo_info);
		void free(const MeshBOInfo& bo_info);
		
		~_MeshBufferManager();
};

class GLOOBufferedMesh : boost::noncopyable {
	private:
		boost::shared_ptr<_MeshBufferManager::MeshBOInfo> _bo_info;
		GLOOBufferedMesh(unsigned int vertexCount, unsigned int faceCount);
	
	public:
		static boost::shared_ptr<GLOOBufferedMesh> create(unsigned int vertexCount, unsigned int faceCount);
		
		// These functions can only be called before the finish_loading() method is called
		void load_vertex(const GLOOVertex& v);
		void load_face(const GLOOFace& f);
		
		// Once you're done calling the load_* methods, call this method to make the Mesh ready to draw
		void finish_loading();
		
		// Call this method to draw the mesh, after finish_loading() has been called
		void draw();
		
		virtual ~GLOOBufferedMesh();
};

#endif
