/*
background.cpp: Implementation of the Background class
The Background class is responsible for outside lighting and for drawing the Smoke Ring and other distant background objects

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

#include <cmath>
#include <boost/lexical_cast.hpp>
#include <GL/glew.h>
#include <GL/gl.h>

#include "autoxsd/orepkgdesc.h"
#include "background.h"
#include "debug.h"
#include "geometry.h"
#include "gloo.h"

Background::Background(const ORE1::SkySettingsType& sky) : _sky(sky)
{
}

void Background::draw() {
  // TODO Draw distant stars

  // Draw and set up lighting for the star
  // TODO Draw the star itself
  float star_pos[4] = {0.0, 0.0, 0.0, 1.0};
  glLightfv(GL_LIGHT1, GL_POSITION, star_pos);

  // Set up ambient lighting (so that areas not directly lit by the star aren't completely dark)
  float part_ald = std::sqrt(2)*AMB_LIGHT_DIST;
  float pos3[4] = {0.0, AMB_LIGHT_DIST, 0.0, 1.0};
  glLightfv(GL_LIGHT2, GL_POSITION, pos3);
  float pos4[4] = {part_ald, -part_ald, 0.0, 1.0};
  glLightfv(GL_LIGHT3, GL_POSITION, pos4);
  float pos5[4] = {-0.5*part_ald, -part_ald, 0.866*part_ald, 1.0};
  glLightfv(GL_LIGHT4, GL_POSITION, pos5);
  float pos6[4] = {-0.5*part_ald, -part_ald, -0.866*part_ald, 1.0};
  glLightfv(GL_LIGHT5, GL_POSITION, pos6);

  // Draw distant static objects (other bubbles, rocks, megaflora, etc)
}

void Background::to_center_from_game_origin() {
  const float d = STAR_DIST + _sky.orbitDOffset();
  glTranslatef(d*std::sin(-rev2rad(_sky.orbitAngle())), _sky.orbitYOffset(), d*std::cos(-rev2rad(_sky.orbitAngle())));
  // TODO Implement tilt
}
