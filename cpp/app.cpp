/*
app.cpp: Implementation of the App class
App is responsible for the main frame loop and calling all the other parts of the program.

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

#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL/SDL.h>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>
#include <algorithm>

#include "app.h"
#include "constants.h"
#include "debug.h"
#include "display.h"
#include "except.h"
#include "gameplay_mode.h"
#include "geometry.h"
#include "globals.h"
#include "gameobj.h"
#include "mesh.h"
#include "mode.h"
#include "performance.h"
#include "sim.h"
#include "resman.h"

// How many ticks each frame must at least last
const unsigned int MIN_TICKS_PER_FRAME = 1000/MAX_FPS;

void App::frame_loop() {
	unsigned int unsimulated_ticks = 0;
	unsigned int frames_since_perf_display = 0;
	
	while (1) {
		GLint frame_start = SDL_GetTicks();
		
		// Add all new events to the events list for this frame, and quit on QUIT events
		Globals::frame_events.clear();
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			Globals::frame_events.push_back(event);
		  	if (event.type == SDL_QUIT) {
				throw GameQuitException("Closed on quit event");
			}
		}
		
		// Do simulation steps until we've no more than one frame behind the display
		while (unsimulated_ticks > MIN_TICKS_PER_FRAME) {
			Sim::_sim_step();
			Globals::total_steps += 1;
			unsimulated_ticks -= MIN_TICKS_PER_FRAME;
		}
		
		// Re-draw the display
		Display::_draw_frame();
		
		// Sleep if we're running faster than our maximum fps
		unsigned int frame_ticks = SDL_GetTicks() - frame_start;
		if (frame_ticks > 0 && frame_ticks < MIN_TICKS_PER_FRAME) {
			SDL_Delay(MIN_TICKS_PER_FRAME - frame_ticks); // Slow down, buster!
		}
		
		// The time that passed during this frame needs to pass in the simulator next frame
		unsigned int total_ticks = SDL_GetTicks() - frame_start;
		unsimulated_ticks += total_ticks;
		
		// Put this frame into the FPS calculations
		Performance::record_frame(total_ticks, total_ticks - frame_ticks);
		
		//FIXME Should output to display instead of console, and should be possible to turn this on and off
		++frames_since_perf_display;
		if (frames_since_perf_display >= MAX_FPS) {
			frames_since_perf_display = 0;
			Debug::debug_msg(Performance::get_perf_info() + " " + GLOOBufferedMesh::get_usage_info());
		}
	}
}

void App::run(const std::vector<std::string>& args) {
	try {
		Display::_init();
		ResMan::_init("main.ore"); // TODO Allow user-selectable ORE file
		Sim::_init();
		
		// TODO Use a menu and/or command-line arguments to select mission and area
		load_mission(1, 3);
	} catch (const std::exception& e) {
		Debug::error_msg(std::string("Uncaught exception during init: ") + e.what());
		return;
	}
	
	try {
		frame_loop();
	} catch (const GameQuitException& e) {
		// Do nothing.
	} catch (const std::exception& e) {
		Debug::error_msg(std::string("Uncaught exception during run: ") + e.what());
	}
	
	// TODO Deinitialize as well as possible here, to reduce possibility of weird error messages on close
}

void App::load_mission(unsigned int area_num, unsigned int mission_num) {
	const ORE1::PkgDescType* desc = &ResMan::pkg().get_pkg_desc();
	
	const ORE1::AreaType* area = NULL;
	for (ORE1::PkgDescType::AreaConstIterator i = desc->area().begin(); i != desc->area().end(); ++i) {
		if (i->n() == area_num) {
			area = &(*i);
			break;
		}
	}
	if (!area) {
		throw GameException(
			"Unable to load area " + boost::lexical_cast<std::string>(area_num)
		);
	}
	
	const ORE1::MissionType* mission = NULL;
	for (ORE1::AreaType::MissionConstIterator i = area->mission().begin(); i != area->mission().end(); ++i) {
		if (i->n() == mission_num) {
			mission = &(*i);
			break;
		}
	}
	if (!mission) {
		throw GameException(
			"Unable to load mission " + boost::lexical_cast<std::string>(mission_num) + " from area " + boost::lexical_cast<std::string>(area_num)
		);
	}
	
	// Okay, we're now sure we have the area and mission we want. Let's load all the objects.
	Globals::gameobjs.clear();
	for (ORE1::AreaType::ObjConstIterator i = area->obj().begin(); i != area->obj().end(); ++i) {
		Globals::gameobjs.insert(GOMap::value_type(i->objName(), GOFactoryRegistry::create(*i)));
	}
	for (ORE1::MissionType::ObjConstIterator i = mission->obj().begin(); i != mission->obj().end(); ++i) {
		Globals::gameobjs.insert(GOMap::value_type(i->objName(), GOFactoryRegistry::create(*i)));
	}
	
	Globals::mode.reset(new GameplayMode());
}
