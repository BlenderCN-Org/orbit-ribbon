/*
geometry.cpp: Implementation of the geometry-related utility classes and functions

Copyright 2011 David Simon <david.mike.simon@gmail.com>

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

//#include <GL/glew.h>
#include <boost/format.hpp>
#include <cmath>
#include <string>
#include <sstream>

#include "geometry.h"

const float SIMILARITY_DELTA = 0.0001;

float rev2rad(float ang) {
  return 2.0*ang*M_PI;
}

float rad2rev(float ang) {
  return ang/(M_PI*2.0);
}

float rev2deg(float ang) {
  return 360.0*ang;
}

float deg2rev(float ang) {
  return ang/360.0;
}

float rad2deg(float ang) {
  return (360.0*ang)/(M_PI*2);
}

float deg2rad(float ang) {
  return (ang/360.0)*2*M_PI;
}

bool similar(float a, float b) {
  return std::fabs(a - b) < SIMILARITY_DELTA;
}

float limit_abs(float x, float max) {
  if (x > max) {
    return max;
  } else if (x < -max) {
    return -max;
  } else {
    return x;
  }
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

Point Point::operator*(float f) const {
  return Point(x*f, y*f, z*f);
}

void Point::operator*=(const Point& other) {
  x *= other.x;
  y *= other.y;
  z *= other.z;
}

void Point::operator*=(float f) {
  x *= f;
  y *= f;
  z *= f;
}

Point Point::operator/(const Point& other) const {
  return Point(x/other.x, y/other.y, z/other.z);
}

Point Point::operator/(float f) const {
  return Point(x/f, y/f, z/f);
}

void Point::operator/=(const Point& other) {
  x /= other.x;
  y /= other.y;
  z /= other.z;
}

void Point::operator/=(float f) {
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

float Point::mag() const {
  return std::sqrt(pow(std::sqrt(x*x + y*y), 2) + z*z);
}

float Point::dot_prod(const Point& other) const {
  return x*other.x + y*other.y + z*other.z;
}

Point Point::cross_prod(const Point& other) const {
  return Point(
    y*other.z - z*other.y,
    z*other.x - x*other.z,
    x*other.y - y*other.x
  );
}

float Point::dist_to(const Point& other) const {
  return std::sqrt(sq_dist_to(other));
}

float Point::sq_dist_to(const Point& other) const {
  return (x-other.x)*(x-other.x) + (y-other.y)*(y-other.y) + (z-other.z)*(z-other.z);
}

float Point::ang_to(const Point& other) const {
  return rad2rev(std::acos((dot_prod(other))/(mag()*other.mag())));
}

Point Point::to_length(float len) const {
  float old_len = mag();
  if (old_len != 0.0) {
    return (*this) * (len/old_len);
  } else {
    return *this;
  }
}

Point Point::project_onto(const Point& other) const {
  return other*(this->dot_prod(other));
}

Point Point::project_onto(const Plane& p) const {
  Vector n = p.normal();
  float dist = this->dot_prod(n) + p.d;
  return *this - (n*dist);
}

Plane::Plane(float na, float nb, float nc, float nd) :
  a(na), b(nb), c(nc), d(nd)
{}

Plane::Plane(const Point& p0, const Point& p1, const Point& p2) {
  Vector v = p1-p0;
  Vector u = p2-p0;
  Vector n = v.cross_prod(u).to_length(1.0);
  a = n.x;
  b = n.y;
  c = n.z;
  d = (-n).dot_prod(p0);
}

Vector Plane::normal() const {
  return Vector(a, b, c);
}

bool Box::contains_point(const Point& pt) const {
  return (
    pt.x >= top_left.x &&
    pt.y >= top_left.y &&
    pt.x < top_left.x + size.x &&
    pt.y < top_left.y + size.y
  );
}

Box Box::operator*(float v) const {
  Box r = *this;
  r.size *= v;
  r.top_left += (size - r.size)/2;
  return r;
}

void Box::operator*=(float v) {
  *this = (*this)*v;
}

Box Box::operator/(float v) const {
  return (*this)*(1.0/v);
}

void Box::operator/=(float v) {
  *this = (*this)/v;
}

Vector get_barycentric(const Point& p, const Point& a, const Point& b, const Point& c) {
  Plane tri_p(a, b, c);
  Point pp = p.project_onto(tri_p);
  
  // Algorithm from Christer Ericson on the gamedev.net forums
  Vector n = tri_p.normal();
  float area_abc = n.dot_prod((b-a).cross_prod(c-a));
  float area_pbc = n.dot_prod((b-p).cross_prod(c-p));
  float area_pca = n.dot_prod((c-p).cross_prod(a-p));
  float ba = area_pbc/area_abc;
  float bb = area_pca/area_abc;
  return Vector(ba, bb, 1.0 - ba - bb);
}
