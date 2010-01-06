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
	float x, y, z;
	float nx, ny, nz;
	float u0, v0;
	float u1, t1;
	float u2, t2;
};

class GLOOVertexBuffer : boost::noncopyable {
	private:
		GLuint _buf_name;
		GLOOVertexBuffer(GLfloat* vbo_data, GLuint vbo_count);
	
	public:
		static boost::shared_ptr<GLOOVertexBuffer> create(GLfloat* data, GLuint count);
		
		virtual ~GLOOVertexBuffer();
};

#endif
