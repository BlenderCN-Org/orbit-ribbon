/*
background.cpp: Implementation of the Background class
The Background class is responsible for outside lighting and for drawing the Smoke Ring and other distant background objects

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

#include <cmath>
#include <boost/foreach.hpp>
#include <boost/random.hpp>
//#include <GL/glew.h>

#include "autoxsd/orepkgdesc.h"
#include "background.h"
#include "except.h"
#include "geometry.h"

#include "mesh.h"

// Settings for the main star
const float STAR_DIST = 5e10; // Distance from center of star to densest part of the Ribbon belt 
const float STAR_RADIUS = 1e9; // Radius of the star itself
const float STAR_COLOR[4] = {1.0, 1.0, 1.0, 1.0};
const float STAR_LIGHT_DIFFUSE[4] = {1.0, 1.0, 1.0, 1.0};

// Ambient light settings
const float AMB_LIGHT_DIST = 2e11; // Distance to the ambient light (not too important, as there's no attenuation)
const float PART_ALD = std::sqrt(2)*AMB_LIGHT_DIST;
const float AMB_LIGHT_DIFFUSE[4] = {0.1, 0.1, 0.1, 1.0}; // Diffuse color of the ambient light

// Starbox
const float STARBOX_DIST = 1e12;

// Settings for generation of ribbon background stuff (i.e. distant bubbles)
const unsigned int RANDOM_STUFF_MASTER_SEED = 827342;
const unsigned int RANDOM_STUFF_SEED_COEF = 67344;
std::vector<Background::RandomStuffDensityRange> Background::density_ranges;

void Background::init() {
  glEnable(GL_LIGHT1); glLightfv(GL_LIGHT1, GL_DIFFUSE, STAR_LIGHT_DIFFUSE);
  glEnable(GL_LIGHT2); glLightfv(GL_LIGHT2, GL_DIFFUSE, AMB_LIGHT_DIFFUSE);
  glEnable(GL_LIGHT3); glLightfv(GL_LIGHT3, GL_DIFFUSE, AMB_LIGHT_DIFFUSE);
  glEnable(GL_LIGHT4); glLightfv(GL_LIGHT4, GL_DIFFUSE, AMB_LIGHT_DIFFUSE);
  glEnable(GL_LIGHT5); glLightfv(GL_LIGHT5, GL_DIFFUSE, AMB_LIGHT_DIFFUSE);
  
  density_ranges.push_back(RandomStuffDensityRange(100, 200, 2e8, 4e8, 1e9, 1e9));
}

void Background::deinit() {
  density_ranges.clear();
  
  glDisable(GL_LIGHT1);
  glDisable(GL_LIGHT2);
  glDisable(GL_LIGHT3);
  glDisable(GL_LIGHT4);
  glDisable(GL_LIGHT5);
}

Background::Background() :
  _star_tex(GLOOTexture::load("star.png")),
  _distant_bubble(MeshAnimation::load("mesh-LIBDistantBubble"))
{
  _starbox_faces.push_back(GLOOTexture::load("starmap-1.png"));
  _starbox_faces.push_back(GLOOTexture::load("starmap-2.png"));
  _starbox_faces.push_back(GLOOTexture::load("starmap-3.png"));
  _starbox_faces.push_back(GLOOTexture::load("starmap-4.png"));
  _starbox_faces.push_back(GLOOTexture::load("starmap-5.png"));
  _starbox_faces.push_back(GLOOTexture::load("starmap-6.png"));
}

void Background::set_sky(const ORE1::SkySettingsType& sky) {
  _sky.reset(new ORE1::SkySettingsType(sky));
  float d = STAR_DIST + sky.orbitDOffset();
  _sky_offset.x = d*std::sin(-rev2rad(sky.orbitAngle()));
  _sky_offset.y = sky.orbitYOffset();
  _sky_offset.z = d*std::cos(-rev2rad(sky.orbitAngle()));
}

void Background::draw_starbox() {
  GLOOPushedMatrix pm;

  // Remove translation; stars are so far away that movement shouldn't change their apparent location
  float modelview[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
  modelview[12] = modelview[13] = modelview[14] = 0.0;
  glLoadMatrixf(modelview);

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

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
}

void Background::draw_objects() {
  // Draw this system's star
  glDisable(GL_LIGHTING);
  glEnable(GL_POINT_SPRITE);
  _star_tex->bind();
  glPointSize(STAR_RADIUS*2);
  glBegin(GL_POINTS);
  glVertex3f(0,0,0);
  glEnd();
  glDisable(GL_POINT_SPRITE);
  glEnable(GL_LIGHTING);

  // Set up lighting for the star
  float star_pos[4] = {0.0, 0.0, 0.0, 1.0};
  glLightfv(GL_LIGHT1, GL_POSITION, star_pos);

  // Set up ambient lighting (so that areas not directly lit by the star aren't completely dark)
  float pos3[4] = {0.0, AMB_LIGHT_DIST, 0.0, 1.0};
  glLightfv(GL_LIGHT2, GL_POSITION, pos3);
  float pos4[4] = {PART_ALD, -PART_ALD, 0.0, 1.0};
  glLightfv(GL_LIGHT3, GL_POSITION, pos4);
  float pos5[4] = {-0.5*PART_ALD, -PART_ALD, 0.866*PART_ALD, 1.0};
  glLightfv(GL_LIGHT4, GL_POSITION, pos5);
  float pos6[4] = {-0.5*PART_ALD, -PART_ALD, -0.866*PART_ALD, 1.0};
  glLightfv(GL_LIGHT5, GL_POSITION, pos6);

  // Draw distant objects
  glEnable(GL_RESCALE_NORMAL);
  unsigned int seed_offset = RANDOM_STUFF_MASTER_SEED;
  BOOST_FOREACH(const RandomStuffDensityRange& r, density_ranges) {
    boost::binomial_distribution<unsigned int, float> count_dist(r.total_count, 1.0/r.segments);
    boost::variate_generator<boost::taus88&, boost::binomial_distribution<unsigned int, float> > count_die(_random_gen, count_dist);

    boost::uniform_real<float> radius_dist(r.rad_min, r.rad_max);
    boost::variate_generator<boost::taus88&, boost::uniform_real<float> > radius_die(_random_gen, radius_dist);

    boost::uniform_real<float> pos_dist(0.0, 1.0/r.segments);
    boost::variate_generator<boost::taus88&, boost::uniform_real<float> > pos_die(_random_gen, pos_dist);

    boost::normal_distribution<float> d_offset_dist(0, r.d_sigma);
    boost::variate_generator<boost::taus88&, boost::normal_distribution<float> > d_offset_die(_random_gen, d_offset_dist);
    
    boost::normal_distribution<float> y_offset_dist(0, r.d_sigma);
    boost::variate_generator<boost::taus88&, boost::normal_distribution<float> > y_offset_die(_random_gen, y_offset_dist);

    static const boost::uniform_real<float> angle_dist(0, 1);
    static const boost::variate_generator<boost::taus88&, boost::uniform_real<float> > angle_die(_random_gen, angle_dist);
    
    for (unsigned int s = 0; s < r.segments; ++s) {
      _random_gen.seed((seed_offset + s)*RANDOM_STUFF_SEED_COEF);
      unsigned int count = count_die();
      float start_angle = s*(1.0/r.segments);
      for (unsigned int i = 0; i < count; ++i) {
        GLOOPushedMatrix pm;
        float pos_angle = rev2rad(start_angle + pos_die());
        float d = STAR_DIST + d_offset_die();
        glTranslatef(d*std::sin(pos_angle), y_offset_die(), d*std::cos(pos_angle));
        float scale = radius_die();
        glScalef(scale, scale, scale);
        _distant_bubble->draw();
      }
    }
    seed_offset += r.segments;
  }
  glDisable(GL_RESCALE_NORMAL);
}

Vector Background::to_center_from_game_origin() {
  if (!_sky) {
    throw GameException("Unable to locate game origin, no SkySettings available");
  }
  return _sky_offset;
}
