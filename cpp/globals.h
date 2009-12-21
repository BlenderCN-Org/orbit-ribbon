#ifndef ORBIT_RIBBON_GLOBAL_H
#define ORBIT_RIBBON_GLOBAL_H

#include <SDL/SDL.h>
#include <vector>

class Globals {
	public:
		static std::vector<SDL_Event> frame_events;
		static GLint total_steps;
};

#endif