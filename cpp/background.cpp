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
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "autoxsd/orepkgdesc.h"
#include "background.h"
#include "geometry.h"
#include "gloo.h"

SkySettings::SkySettings() :
  orbit_angle(0),
  orbit_y_offset(0),
  orbit_d_offset(0),
  tilt_angle(0),
  tilt_x(0),
  tilt_z(0),
  bubble_radius(0)
{}

SkySettings::SkySettings(const boost::array<float, 10>& args) :
  orbit_angle(args[0]),
  orbit_y_offset(args[1]),
  orbit_d_offset(args[2]),
  tilt_angle(args[3]),
  tilt_x(args[4]),
  tilt_z(args[5]),
  bubble_radius(args[6]),
  bubble_pos(args[7], args[8], args[9])
{}

SkySettings::SkySettings(const ORE1::SkySettingsType& area) :
  orbit_angle(area.orbitAngle()),
  orbit_y_offset(area.orbitYOffset()),
  orbit_d_offset(area.orbitDOffset()),
  tilt_angle(area.tiltAngle()),
  tilt_x(area.tiltX()),
  tilt_z(area.tiltZ()),
  bubble_radius(area.bubbleRadius())
{}

void SkySettings::fill_array(boost::array<float, 10>& tgt) {
  tgt[0] = orbit_angle;
  tgt[1] = orbit_y_offset;
  tgt[2] = orbit_d_offset;
  tgt[3] = tilt_angle;
  tgt[4] = tilt_x;
  tgt[5] = tilt_z;
  tgt[6] = bubble_radius;
  tgt[7] = bubble_pos.x;
  tgt[8] = bubble_pos.y;
  tgt[9] = bubble_pos.z;
}

Point Background::get_game_origin() {
  const float d = STAR_DIST + _sky.orbit_d_offset;
  // TODO : Check if this matches the part below that figures out where to put the light origin of the star and draw it
  return -Point(d*std::sin(-rev2rad(_sky.orbit_angle)), _sky.orbit_y_offset, d*std::cos(-rev2rad(_sky.orbit_angle)));
}

Point Background::convert_to_sky_coords(const Point& pt) {
  //TODO Implement
}

Background::Background(const SkySettings& sky) {
  set_sky(sky);
  _bubble_quadric = gluNewQuadric();
}

void Background::set_sky(const SkySettings& sky) {
  _sky = sky;
  
  // Generate the transformation matrix that moves us from the game origin to the star
  // TODO Probably more efficient to do this without using OpenGL, but this sure is convenient
  {
    GLOOPushedMatrix pm;
    glLoadIdentity();
    glRotatef(_sky.tilt_angle, _sky.tilt_x, 0, _sky.tilt_z); // Apply tilt
    glTranslatef(0.0, -_sky.orbit_y_offset, STAR_DIST + _sky.orbit_d_offset); // Move out to the star
    glRotatef(rev2deg(_sky.orbit_angle), 0, 1, 0); // Rotate the Ribbon around the star
    glGetFloatv(GL_MODELVIEW_MATRIX, _skyMatr.begin());
  }
}

void Background::set_clear_color() {
  // TODO Have this change based on atmospheric thickness; actually, better yet to make it part of bubble drawing
  glClearColor(0.6, 0.6, 1.0, 0.0);
}

void Background::draw() {
  {
    // Draw the bubble's inside and outside surface
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE);
    // TODO Draw the outside
    // Inside
    gluQuadricOrientation(_bubble_quadric, GLU_INSIDE);
    glColor4f(1.0, 0.5, 0.2, 0.5);
    //glTranslatef(_sky.bubble_pos.x, _sky.bubble_pos.y, _sky.bubble_pos.z);
    gluSphere(_bubble_quadric, _sky.bubble_radius, 32, 32);
    glEnable(GL_TEXTURE);
    glEnable(GL_LIGHTING);
  }

  {
    GLOOPushedMatrix pm;
    glMultMatrixf(_skyMatr.begin());

    // TODO Draw stars

    // Draw and set up lighting for the star
    // TODO Draw the star itself
    float star_pos[4] = {std::sin(-rev2rad(_sky.orbit_angle))*STAR_DIST, 0.0, std::cos(-rev2rad(_sky.orbit_angle))*STAR_DIST, 1.0};
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

    // Draw distant objects (other bubbles, rocks, megafauna, etc)
  }
}
