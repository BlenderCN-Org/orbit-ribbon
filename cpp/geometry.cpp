#include <GL/gl.h>

#include <cmath>
#include <string>
#include <sstream>

#include "geometry.h"

const GLfloat near_to_delta = 0.0001;

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

Point& Point::operator=(const Point& other) {
	x = other.x;
	y = other.y;
	z = other.z;
	return *this;
}

std::string Point::to_str() const {
	// FIXME : Implement
	return std::string("");
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
		std::abs(x - other.x) < near_to_delta &&
		std::abs(y - other.y) < near_to_delta &&
		std::abs(z - other.z) < near_to_delta
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

Point Point::to_length(GLfloat len) const {
	GLfloat old_len = mag();
	if (old_len != 0.0) {
		return (*this) * (len/old_len);
	} else {
		return *this;
	}
}
