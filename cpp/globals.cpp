/*
globals.cpp: Translation unit for game-wide globals

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

#include <SDL/SDL.h>

#include "background.h"
#include "font.h"
#include "gameobj.h"
#include "globals.h"
#include "gloo.h"
#include "mode.h"
#include "ore.h"
#include "mouse_cursor.h"
#include "autoxsd/orepkgdesc.h"
#include "autoxsd/save.h"

std::vector<SDL_Event> Globals::frame_events;
unsigned int Globals::total_steps = 0;
GOMap Globals::gameobjs;
ModeStack Globals::mode_stack;
boost::filesystem::path Globals::save_dir;
boost::scoped_ptr<Background> Globals::bg;
boost::scoped_ptr<Font> Globals::sys_font;
boost::scoped_ptr<OrePackage> Globals::ore;
boost::scoped_ptr<MouseCursor> Globals::mouse_cursor;
const ORE1::AreaType* Globals::current_area = NULL;
const ORE1::MissionType* Globals::current_mission = NULL;
LSMap Globals::libscenes;
