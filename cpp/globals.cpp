#include <GL/gl.h>
#include <SDL/SDL.h>

#include "globals.h"

GLint Globals::total_steps = 0;
std::vector<SDL_Event> Globals::frame_events;