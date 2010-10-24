/*
bubble.h: Header of the Bubble class
Bubble is a GameObject representing the bubble that contains the atmosphere

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
#include <GL/glu.h>

#include "bubble.h"
#include "autoxsd/orepkgdesc.h"

AutoRegistration<GameObjFactorySpec, BubbleGameObj> bubble_gameobj_reg("Bubble");

void BubbleGameObj::near_draw_impl() {
  // Draw the bubble's inside and/or outside surface
  
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  
  // TODO Draw the outside
  
  // Inside
  gluQuadricOrientation(_quadric, GLU_INSIDE);
  glColor4f(0.7, 0.7, 1.0, 0.8);
  gluSphere(_quadric, _radius, 16, 16);
  
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_LIGHTING);
}

BubbleGameObj::BubbleGameObj(const ORE1::ObjType& obj) :
  GameObj(obj),
  _quadric(gluNewQuadric()),
  _radius(static_cast<const ORE1::BubbleObjType&>(obj).radius())
{
}
