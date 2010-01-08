/*
constants.h: Header defining game-wide constants

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

// String describing the current version of Orbit Ribbon
const char* const APP_VERSION = "prealpha";

// Maximum number of frames per second, and the base number of simulated steps per second
const int MAX_FPS = 60;

// Default coefficients for linear and angular damping on new GameObjs
const float DEFAULT_VEL_DAMP_COEF = 0.15;
const float DEFAULT_ANG_DAMP_COEF = 0.15;

// How far from -1, 0, or 1 where we consider an input axis to just be at those exact values
const float DEAD_ZONE = 0.001;

// Space separated list of key names to ignore in the input module because they cause problems
const char* const IGNORE_KEYS = "[-] numlock";

// How many bytes to load from ORE files in each chunk
const int ORE_CHUNK_SIZE = 4096;

// How many kilobytes should be allocated for the VBO and IBO respectively
const int VBO_ALLOCATED_SIZE = 4096;
const int IBO_ALLOCATED_SIZE = 1024;

#endif
