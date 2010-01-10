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
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>
#include <string>
#include <list>
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include "cache.h"

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
		std::string _load_name;
		GLuint _tex_name;
		GLuint _width, _height;
		
		GLOOTexture() : _tex_name(0) {}
		
		friend class _TextureCache;
	
	public:
		static boost::shared_ptr<GLOOTexture> load(const std::string& name);
		
		std::string get_load_name() { return _load_name; }
		GLuint get_width() { return _width; }
		GLuint get_height() { return _height; }
		
		void bind();
		
		virtual ~GLOOTexture();
};

struct GLOOVertex {
	GLfloat x, y, z;
	GLfloat nx, ny, nz;
	GLfloat u, v;
	
	static void set_gl_pointers();
};

struct GLOOFace {
	// Indices to the vertices forming a triangle face
	GLuint a, b, c;
		
	static GLenum gl_type() { return GL_UNSIGNED_INT; }
	static unsigned int elem_bytes() { return sizeof(GLuint); }
};

class _VBOManager;
class _VBOManager : boost::noncopyable {
	private:
		struct AllocRange {
			unsigned int offset, bytes;
		};
		
		GLenum _tgt;
		GLuint _buf_id;
		unsigned int _max_bytes;
		bool _mapped;
		std::list<AllocRange> _ranges;
	
	public:
		class Allocation;
		friend class Allocation;
		class Allocation : boost::noncopyable {
			private:
				std::list<AllocRange>::iterator _iter;
				_VBOManager* _man;
				
				Allocation(unsigned int length, _VBOManager* man);
				
				friend class _VBOManager;
			
			public:
				unsigned int offset() { return _iter->offset; }
				unsigned int bytes() { return _iter->bytes; }
				void* map();
				void unmap();
				
				~Allocation();
		};
		
		_VBOManager(GLenum tgt, unsigned int max_bytes);
		~_VBOManager();
		
		boost::shared_ptr<Allocation> allocate(unsigned int bytes)
			{ return boost::shared_ptr<Allocation>(new Allocation(bytes, this)); }
			
		float get_usage();
		
		bool mapped() { return _mapped; }
};

class GLOOBufferedMesh : boost::noncopyable {
	private:
		static bool _initialized;
		static boost::scoped_ptr<_VBOManager> _vertices_vboman, _faces_vboman;
		
		boost::shared_ptr<GLOOTexture> _tex;
		boost::shared_ptr<_VBOManager::Allocation> _vertices_alloc, _faces_alloc;
		unsigned int _vertices_added, _total_vertices, _faces_added, _total_faces;
		GLOOVertex* _next_vertex;
		GLOOFace* _next_face;
		
		GLOOBufferedMesh(unsigned int vertex_count, unsigned int face_count, boost::shared_ptr<GLOOTexture> tex);
	
	public:
		static boost::shared_ptr<GLOOBufferedMesh> create(unsigned int vertex_count, unsigned int face_count, boost::shared_ptr<GLOOTexture> tex)
			{ return boost::shared_ptr<GLOOBufferedMesh>(new GLOOBufferedMesh(vertex_count, face_count, tex)); }
			
		static float get_vertices_usage() { return _vertices_vboman ? _vertices_vboman->get_usage() : 0.0; }
		static float get_faces_usage() { return _faces_vboman ? _faces_vboman->get_usage() : 0.0; }
		
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
