/*
geometry.h: Header for the geometry-related utility classes and functions

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

#ifndef ORBIT_RIBBON_GEOMETRY_H
#define ORBIT_RIBBON_GEOMETRY_H

#include <GL/gl.h>

#include <string>

GLfloat rev2rad(GLfloat ang); // FIXME: In old version, this also converted cw to ccw. Make sure everything that needed that does it itself!
GLfloat rad2rev(GLfloat ang);
GLfloat rev2deg(GLfloat ang);
GLfloat deg2rev(GLfloat ang);
GLfloat rad2deg(GLfloat ang);
GLfloat deg2rad(GLfloat ang);

class Point {
	public:
		GLfloat x;
		GLfloat y;
		GLfloat z;
		
		Point(GLfloat nx = 0.0, GLfloat ny = 0.0, GLfloat nz = 0.0) : x(nx), y(ny), z(nz) {}
		Point(const Point& other) : x(other.x), y(other.y), z(other.z) {}
		
		Point& operator=(const Point& other);
		
		Point operator+(const Point& other) const;
		void operator+=(const Point& other);
		
		Point operator-(const Point& other) const;
		void operator-=(const Point& other);
		
		Point operator*(const Point& other) const;
		Point operator*(GLfloat f) const;
		void operator*=(const Point& other);
		void operator*=(GLfloat f);
		
		Point operator/(const Point& other) const;
		Point operator/(GLfloat f) const;
		void operator/=(const Point& other);
		void operator/=(GLfloat f);
		
		bool operator==(const Point& other) const;
		bool operator!=(const Point& other) const { return !(*this == other); }
		
		Point operator-() const;
		
		bool near_to(const Point& other) const;
		GLfloat dot_prod(const Point& other) const;
		GLfloat mag() const;
		GLfloat dist_to(const Point& other) const;
		GLfloat ang_to(const Point& other) const;
		Point to_length(GLfloat len) const;
		
		std::string to_str() const;
};

typedef Point Vector;
typedef Point Size;

#endif
