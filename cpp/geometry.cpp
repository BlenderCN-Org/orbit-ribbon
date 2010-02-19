/*
geometry.cpp: Implementation of the geometry-related utility classes and functions

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

#include <GL/glew.h>
#include <GL/gl.h>
#include <boost/format.hpp>
#include <cmath>
#include <string>
#include <sstream>

#include "geometry.h"

const float SIMILARITY_DELTA = 0.0001;

GLfloat rev2rad(GLfloat ang) {
	return 2.0*ang*M_PI;
}

GLfloat rad2rev(GLfloat ang) {
	return ang/(M_PI*2.0);
}

GLfloat rev2deg(GLfloat ang) {
	return 360.0*ang;
}

GLfloat deg2rev(GLfloat ang) {
	return ang/360.0;
}

GLfloat rad2deg(GLfloat ang) {
	return (360.0*ang)/(M_PI*2);
}

GLfloat deg2rad(GLfloat ang) {
	return (ang/360.0)*2*M_PI;
}

bool similar(float a, float b) {
	return std::fabs(a - b) < SIMILARITY_DELTA;
}

Point& Point::operator=(const Point& other) {
	x = other.x;
	y = other.y;
	z = other.z;
	return *this;
}

std::string Point::to_str() const {
	return (boost::format("%.3f, %.3f, %.3f") % x % y % z).str();
}

Point Point::operator+(const Point& other) const {
	return Point(x + other.x, y + other.y, z + other.z);
}

void Point::operator+=(const Point& other) {
	x += other.x;
	y += other.y;
	z += other.z;
}

Point Point::operator-(const Point& other) const {
	return Point(x - other.x, y - other.y, z - other.z);
}

void Point::operator-=(const Point& other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;
}

Point Point::operator*(const Point& other) const {
	return Point(x*other.x, y*other.y, z*other.z);
}

Point Point::operator*(GLfloat f) const {
	return Point(x*f, y*f, z*f);
}

void Point::operator*=(const Point& other) {
	x *= other.x;
	y *= other.y;
	z *= other.z;
}

void Point::operator*=(GLfloat f) {
	x *= f;
	y *= f;
	z *= f;
}

Point Point::operator/(const Point& other) const {
	return Point(x/other.x, y/other.y, z/other.z);
}

Point Point::operator/(GLfloat f) const {
	return Point(x/f, y/f, z/f);
}

void Point::operator/=(const Point& other) {
	x /= other.x;
	y /= other.y;
	z /= other.z;
}

void Point::operator/=(GLfloat f) {
	x /= f;
	y /= f;
	z /= f;
}

bool Point::operator==(const Point& other) const {
	return x == other.x && y == other.y && z == other.z;
}

Point Point::operator-() const {
	return Point(-x, -y, -z);
}

bool Point::near_to(const Point& other) const {
	return (
		similar(x, other.x) &&
		similar(y, other.y) &&
		similar(z, other.z)
	);
}

GLfloat Point::mag() const {
	return std::sqrt(pow(std::sqrt(x*x + y*y), 2) + z*z);
}

GLfloat Point::dot_prod(const Point& other) const {
	return x*other.x + y*other.y + z*other.z;
}

GLfloat Point::dist_to(const Point& other) const {
	return std::sqrt(pow(std::sqrt(pow(x-other.x, 2) + pow(y-other.y, 2)), 2) + pow(z-other.z, 2));
}

GLfloat Point::ang_to(const Point& other) const {
	return rad2rev(std::acos((dot_prod(other))/(mag()*other.mag())));
}

Point Point::to_length(float len) const {
	GLfloat old_len = mag();
	if (old_len != 0.0) {
		return (*this) * (len/old_len);
	} else {
		return *this;
	}
}

Point Point::project_onto(const Point& other) const {
	return other*(this->dot_prod(other));
}