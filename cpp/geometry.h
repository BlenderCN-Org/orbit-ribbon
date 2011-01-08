/*
geometry.h: Header for the geometry-related utility classes and functions

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

#ifndef ORBIT_RIBBON_GEOMETRY_H
#define ORBIT_RIBBON_GEOMETRY_H

#include <string>
#include <ode/ode.h>

class Point;
class Plane;
typedef Point Vector;
typedef Point Size;

class Point {
  public:
    float x;
    float y;
    float z;
    
    Point(const dReal* s) : x(s[0]), y(s[1]), z(s[2]) {}
    Point(float nx = 0.0, float ny = 0.0, float nz = 0.0) : x(nx), y(ny), z(nz) {}
    Point(const Point& other) : x(other.x), y(other.y), z(other.z) {}
    
    float operator[](int i) const { return (i == 0 ? x : (i == 1 ? y : z)); }
    
    Point& operator=(const Point& other);
    
    Point operator+(const Point& other) const;
    void operator+=(const Point& other);
    
    Point operator-(const Point& other) const;
    void operator-=(const Point& other);
    
    Point operator*(const Point& other) const;
    Point operator*(float f) const;
    void operator*=(const Point& other);
    void operator*=(float f);
    
    Point operator/(const Point& other) const;
    Point operator/(float f) const;
    void operator/=(const Point& other);
    void operator/=(float f);
    
    bool operator==(const Point& other) const;
    bool operator!=(const Point& other) const { return !(*this == other); }
    
    Point operator-() const;
    
    bool near_to(const Point& other) const;
    float dot_prod(const Point& other) const;
    Point cross_prod(const Point& other) const;
    float mag() const;
    float dist_to(const Point& other) const;
    float sq_dist_to(const Point& other) const;
    float ang_to(const Point& other) const;
    Point to_length(float len) const;
    Point project_onto(const Point& other) const;
    Point project_onto(const Plane& other) const;
    
    std::string to_str() const;
};

class Plane {
  public:
    float a, b, c, d;
    
    Plane(float na, float nb, float nc, float nd);
    Plane(const Point& p0, const Point& p1, const Point& p2);
    
    Vector normal() const;
};

struct Box {
  Point top_left;
  Size size;
  
  Box() {}
  Box(const Point& tl, const Size& s) : top_left(tl), size(s) {}
  
  bool contains_point(const Point& pt) const;
};

float rev2rad(float ang); // FIXME: In old version, this also converted cw to ccw. Make sure everything that needed that does it itself!
float rad2rev(float ang);
float rev2deg(float ang);
float deg2rev(float ang);
float rad2deg(float ang);
float deg2rad(float ang);
bool similar(float a, float b);
float limit_abs(float x, float max);
Vector get_barycentric(const Point& p, const Point& a, const Point& b, const Point& c);

#endif
