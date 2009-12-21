#include <GL/gl.h>
#include <SDL/SDL.h>
#include <boost/ptr_container/ptr_vector.hpp>

#include "gameobj.h"
#include "globals.h"

GLint Globals::total_steps = 0;
std::vector<SDL_Event> Globals::frame_events;
boost::ptr_vector<GameObj> Globals::gameobjs;