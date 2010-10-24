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

#include "geometry.h"

// Settings for the star
const float STAR_DIST = 5e10; // Distance from center of star to densest part of the Ribbon belt 
const float STAR_RADIUS = 1e9; // Radius of the star itself
const float STAR_COLOR[4] = {1.0, 1.0, 1.0, 1.0};
const float STAR_LIGHT_DIFFUSE[4] = {1.0, 1.0, 1.0, 1.0};

// Ambient light settings
const float AMB_LIGHT_DIST = 2e11; // Distance to the ambient light (not too important, as there's no attenuation)
const float AMB_LIGHT_DIFFUSE[4] = {0.1, 0.1, 0.1, 1.0}; // Diffuse color of the ambient light

struct GLUquadric;
namespace ORE1 { class SkySettingsType; }

class Background {
  private:
    const ORE1::SkySettingsType& _sky;
    GLUquadric* _quadric;
    
  public:
    Background(const ORE1::SkySettingsType& sky);
    
    void draw();
    void to_center_from_game_origin();
};

#endif
