/*
bubble.cpp: Implementation of the Bubble class
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

#include "bubble.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

// For now, no auto registration; Bubble is to be created manually during area instantiation

void BubbleGameObj::near_draw_impl() {
  // Draw the bubble's inside and outside surface
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE);
  // TODO Draw the outside
  // Inside
  gluQuadricOrientation(_quadric, GLU_INSIDE);
  glColor4f(1.0, 0.5, 0.2, 0.5);
  //glTranslatef(_sky.bubble_pos.x, _sky.bubble_pos.y, _sky.bubble_pos.z);
  gluSphere(_quadric, _radius, 32, 32);
  glEnable(GL_TEXTURE);
  glEnable(GL_LIGHTING);
}

BubbleGameObj::BubbleGameObj(const Point& pos, float radius) :
  GameObj(pos),
  _quadric(gluNewQuadric()),
  _radius(radius)
{
}