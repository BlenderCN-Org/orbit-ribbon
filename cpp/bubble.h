/*
bubble.h: Header of the Bubble class
Bubble is a GameObject representing the surface of a large bubble surrounding an asteroid

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

#ifndef ORBIT_RIBBON_BUBBLE_H
#define ORBIT_RIBBON_BUBBLE_H

#include "gameobj.h"

class Point;
class GLUquadric;

class BubbleGameObj : public GameObj {
  private:
    GLUquadric* _quadric;
    float _radius;

  protected:
    void near_draw_impl();

  public:
    BubbleGameObj(const Point& pos, float radius);
};

#endif
