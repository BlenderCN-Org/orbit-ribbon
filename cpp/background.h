/*
background.h: Header of the Background class
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

#ifndef ORBIT_RIBBON_BACKGROUND_H
#define ORBIT_RIBBON_BACKGROUND_H

#include <boost/array.hpp>

#include "geometry.h"

// Settings for the star
const float STAR_DIST = 5e10; // Distance between the center of the star and the center of the Ribbon torus
const float STAR_LIGHT_DIFFUSE[4] = {1.0, 1.0, 1.0, 1.0};

// Ambient light settings
const float AMB_LIGHT_DIST = 1e9; // Distance to the ambient light (not too important, as there's no attenuation)
const float AMB_LIGHT_DIFFUSE[4] = {0.15, 0.15, 0.15, 1.0}; // Diffuse color of the ambient light

namespace ORE1 { class SkySettingsType; }

struct SkySettings {
  float orbit_angle;
  float orbit_y_offset;
  float orbit_d_offset;
  
  float tilt_angle;
  float tilt_x;
  float tilt_z;

  SkySettings();
  SkySettings(const boost::array<float, 6>& args);
  SkySettings(const ORE1::SkySettingsType& area);
  
  void fill_array(boost::array<float, 6>& tgt);
};

class Background {
  private:
    SkySettings _sky;
    boost::array<float, 16> _skyMatr;
    
    Point get_game_origin();
    Point convert_to_sky_coords(const Point& pt);
  
  public:
    Background(const SkySettings& sky);
    
    const SkySettings& get_sky() const { return _sky; }
    void set_sky(const SkySettings& sky);
    
    void set_clear_color();
    void draw();
};

#endif
