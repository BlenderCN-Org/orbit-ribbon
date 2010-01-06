/*
globals.h: Header for game-wide globals

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

#ifndef ORBIT_RIBBON_GLOBALS_H
#define ORBIT_RIBBON_GLOBALS_H

#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL/SDL.h>
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

class GameObj;

class Globals {
	public:
		static std::vector<SDL_Event> frame_events;
		static GLint total_steps;
		static boost::ptr_vector<GameObj> gameobjs;
};

#endif