/*
globals.h: Header for game-wide globals

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

#ifndef ORBIT_RIBBON_GLOBALS_H
#define ORBIT_RIBBON_GLOBALS_H

#include <Horde3D.h>
#include <Horde3DUtils.h>
#include <SDL/SDL.h>
#include <vector>
#include <string>
#include <map>
#include <boost/array.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <ode/ode.h>

#include "geometry.h"
#include "mode.h"

class Background;
class GameObj;
class Mode;
class GLOOFont;
class Font;
class GLOOTexture;
class OrePackage;
class MouseCursor;

namespace ORE1 { class AreaType; class MissionType; class SubsceneType; }

typedef std::map<std::string, boost::shared_ptr<GameObj> > GOMap;
typedef std::map<std::string, const ORE1::SubsceneType&> LSMap;

class Globals {
  public:
    static std::vector<SDL_Event> frame_events;
    static unsigned int total_steps;
    static GOMap gameobjs;
    static boost::scoped_ptr<ModeStack> mode_stack;
    static H3DRes pipeRes;
    static H3DNode cam;
    static boost::filesystem::path save_dir;
    static boost::scoped_ptr<Background> bg;
    static boost::scoped_ptr<Font> sys_font;
    static boost::scoped_ptr<OrePackage> ore;
    static boost::scoped_ptr<MouseCursor> mouse_cursor;
    static const ORE1::AreaType* current_area;
    static const ORE1::MissionType* current_mission;
    static LSMap libscenes;
};

#endif
