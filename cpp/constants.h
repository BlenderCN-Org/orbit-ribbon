/*
constants.h: Header defining constants used in several different modules

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

#ifndef ORBIT_RIBBON_CONSTANTS_H
#define ORBIT_RIBBON_CONSTANTS_H

#include "geometry.h"

// String describing the current version of Orbit Ribbon
const char* const APP_VERSION = "prealpha";

// Maximum number of frames per second, and the base number of simulated steps per second
const unsigned int MAX_FPS = 60;

// How many ticks each frame must at least last
const unsigned int MIN_TICKS_PER_FRAME = 1000/MAX_FPS;

// Clipping distance for gameplay objects and background objects respectively
const float GAMEPLAY_CLIP_DIST = 50000;
const float SKY_CLIP_DIST = 2e12;

// Field-of-view in degrees
const float FOV = 45;

// Number of na'ananvi (year-billionths) per second
const float NANVI_PER_SECOND = 0.6;

#endif
