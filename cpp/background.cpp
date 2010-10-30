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
#include <GL/glu.h>

#include "autoxsd/orepkgdesc.h"
#include "background.h"
#include "debug.h"
#include "except.h"
#include "geometry.h"
#include "gloo.h"

// Settings for the main star
const float STAR_DIST = 5e10; // Distance from center of star to densest part of the Ribbon belt 
const float STAR_RADIUS = 1e9; // Radius of the star itself
const float STAR_COLOR[4] = {1.0, 1.0, 1.0, 1.0};
const float STAR_LIGHT_DIFFUSE[4] = {1.0, 1.0, 1.0, 1.0};

// Ambient light settings
const float AMB_LIGHT_DIST = 2e11; // Distance to the ambient light (not too important, as there's no attenuation)
const float AMB_LIGHT_DIFFUSE[4] = {0.1, 0.1, 0.1, 1.0}; // Diffuse color of the ambient light

// Starbox
const float STARBOX_DIST = 1e12;

void Background::setup_lights() {
  glEnable(GL_LIGHT1); glLightfv(GL_LIGHT1, GL_DIFFUSE, STAR_LIGHT_DIFFUSE);
  glEnable(GL_LIGHT2); glLightfv(GL_LIGHT2, GL_DIFFUSE, AMB_LIGHT_DIFFUSE);
  glEnable(GL_LIGHT3); glLightfv(GL_LIGHT3, GL_DIFFUSE, AMB_LIGHT_DIFFUSE);
  glEnable(GL_LIGHT4); glLightfv(GL_LIGHT4, GL_DIFFUSE, AMB_LIGHT_DIFFUSE);
  glEnable(GL_LIGHT5); glLightfv(GL_LIGHT5, GL_DIFFUSE, AMB_LIGHT_DIFFUSE);
  
}

Background::Background() : _quadric(gluNewQuadric()) {
  _starbox_faces.push_back(GLOOTexture::load("starmap1.png"));
  _starbox_faces.push_back(GLOOTexture::load("starmap2.png"));
  _starbox_faces.push_back(GLOOTexture::load("starmap3.png"));
  _starbox_faces.push_back(GLOOTexture::load("starmap4.png"));
  _starbox_faces.push_back(GLOOTexture::load("starmap5.png"));
  _starbox_faces.push_back(GLOOTexture::load("starmap6.png"));
}

void Background::set_sky(const ORE1::SkySettingsType& sky) {
  _sky.reset(new ORE1::SkySettingsType(sky));
}

void Background::draw() {
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);

  // Draw the starbox
  const static float uv[8] = {
    1.0, 0.0,
    0.0, 0.0,
    0.0, 1.0,
    1.0, 1.0
  };
  const static float d = STARBOX_DIST;
  for (int i = 0; i < 6; ++i) {
    GLOOPushedMatrix pm;
    unsigned int uv_offset = 0;
    switch(i) {
      case 0:
        break;
      case 1:
        glRotatef(-90, 0, 1, 0);
        break;
      case 2:
        glRotatef(180, 0, 1, 0);
        break;
      case 3:
        glRotatef(90, 1, 0, 0);
        uv_offset = 1;
        break;
      case 4:
        glRotatef(90, 0, 1, 0);
        uv_offset = 2;
        break;
      case 5:
        glRotatef(-90, 1, 0, 0);
        uv_offset = 3;
        break;
    }

    _starbox_faces[i]->bind();
    glBegin(GL_QUADS);
    glTexCoord2fv(uv + ((uv_offset+0)%4)*2);
    glVertex3f(-d, +d, +d);
    glTexCoord2fv(uv + ((uv_offset+1)%4)*2);
    glVertex3f(+d, +d, +d);
    glTexCoord2fv(uv + ((uv_offset+2)%4)*2);
    glVertex3f(+d, -d, +d);
    glTexCoord2fv(uv + ((uv_offset+3)%4)*2);
    glVertex3f(-d, -d, +d);
    glEnd();
  }
 
  // Draw this system's star
  glDisable(GL_TEXTURE_2D);
  glColor4fv(STAR_COLOR);
  gluSphere(_quadric, STAR_RADIUS, 16, 16);
  glEnable(GL_TEXTURE_2D);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);

  // Set up lighting for the star
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
  if (!_sky) {
    throw GameException("Unable to locate game origin, no SkySettings available");
  }
  const float d = STAR_DIST + _sky->orbitDOffset();
  glTranslatef(d*std::sin(-rev2rad(_sky->orbitAngle())), _sky->orbitYOffset(), d*std::cos(-rev2rad(_sky->orbitAngle())));
  // TODO Implement tilt
}
