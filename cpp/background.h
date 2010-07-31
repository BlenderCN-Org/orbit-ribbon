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

// Distances in meters from Voy to other sky objects, and sizes of various objects
const float VOY_RADIUS = 2e4; // From book
const float T3_DIST = 2.5e11; // From book
const float T3_RADIUS = 2.5e9; // Our Sun's radius times about 1.2 is 8.4e8, which is from book... but that looks too small
const float TORUS_OUTSIDE_DIST = 1e9; // From book
const float TORUS_INSIDE_DIST = 1e7; // Guessed
const float TORUS_RADIUS = TORUS_OUTSIDE_DIST - TORUS_INSIDE_DIST; // Assuming a circular torus, which probably isn't quite right
const float GOLD_DIST = 2.6e7; // From book; also, assuming that Gold is in precise middle of Smoke Ring
const float GOLD_RADIUS = 5e5; // Guessed; includes the storm around Gold
const float SMOKE_RING_RADIUS = 8e4; // Calculations yielded 1.4e7, but that looked terrible, so this is made up. FIXME Too narrow.
const float SMOKE_RING_INSIDE_DIST = GOLD_DIST - SMOKE_RING_RADIUS;
const float SMOKE_RING_OUTSIDE_DIST = GOLD_DIST + SMOKE_RING_RADIUS;

// T3 light settings
const float T3_LIGHT_DIFFUSE[4] = {1.0, 1.0, 1.0, 1.0};

// Ambient light settings
const float AMB_LIGHT_DIST = 1e10; // Distance to the ambient light (not too important, as there's no attenuation)
const float AMB_LIGHT_DIFFUSE[4] = {0.15, 0.15, 0.15, 1.0}; // Diffuse color of the ambient light

class Point;
namespace ORE1 { class SkySettingsType; }

struct SkySettings {
  float ring_angle;
  float ring_y_offset;
  float ring_d_offset;
  float tilt_angle;
  float tilt_x;
  float tilt_z;
  float t3_angle;
  
  SkySettings();
  SkySettings(const boost::array<float, 7>& args);
  SkySettings(const ORE1::SkySettingsType& area);
  
  void fill_array(boost::array<float, 7>& tgt);
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
    
    float get_dist_from_ring(const Point& pt);
    
    void set_clear_color();
    void draw();
};

#endif
