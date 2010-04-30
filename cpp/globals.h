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

#include <SDL/SDL.h>
#include <vector>
#include <string>
#include <map>
#include <stack>
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/scoped_ptr.hpp>
#include <ode/ode.h>

class GameObj;
class Mode;
class Background;
class GLOOFont;
class OrePackage;

typedef std::map<std::string, boost::shared_ptr<GameObj> > GOMap;

class Globals {
	public:
		static std::vector<SDL_Event> frame_events;
		static unsigned int total_steps;
		static GOMap gameobjs;
		static std::stack<boost::shared_ptr<Mode> > mode_stack;
		static boost::scoped_ptr<Background> bg;
		static boost::scoped_ptr<GLOOFont> sys_font;
		static boost::scoped_ptr<OrePackage> ore;
};

#endif
