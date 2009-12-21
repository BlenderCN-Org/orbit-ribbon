#ifndef ORBIT_RIBBON_GLOBALS_H
#define ORBIT_RIBBON_GLOBALS_H

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