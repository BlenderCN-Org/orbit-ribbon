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
		
		// FIXME
		//bool operator==(const Point& other) const;
		//bool operator!=(const Point& other) const { return !(*this == other); }
		
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
