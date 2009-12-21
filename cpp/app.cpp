#include <GL/gl.h>
#include <SDL/SDL.h>

#include <string>
#include <vector>

#include "app.h"
#include "constants.h"
#include "debug.h"
#include "display.h"
#include "except.h"
#include "globals.h"
#include "sim.h"

void App::frame_loop() {
	const GLint max_ticks_per_frame = 1000/MAX_FPS;
	int unsimulated_ticks = 0;
	
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
		
		// Do simulation steps until we've caught up with the display
		while (unsimulated_ticks > max_ticks_per_frame) {
			Sim::_sim_step();
			Globals::total_steps += 1;
			unsimulated_ticks -= max_ticks_per_frame;
		}
		
		// Re-draw the display
		Display::_draw_frame();
		
		// Sleep if we're running faster than our maximum fps
		GLint frame_ticks = SDL_GetTicks() - frame_start;
		if (frame_ticks > 0 && frame_ticks < max_ticks_per_frame) {
			SDL_Delay(max_ticks_per_frame - frame_ticks); // Slow down, buster!
		}
		
		// The time that passed during this frame needs to pass in the simulator next frame
		unsimulated_ticks += SDL_GetTicks() - frame_start;
	}
}

void App::run(const std::vector<std::string>& args) {
	try {
		Display::_init();
		Sim::_init();
	} catch (GameException e) {
		Debug::error_msg(std::string("Uncaught exception during init: ") + e.get_msg());
		return;
	}
	
	try {
		frame_loop();
	} catch (GameQuitException e) {
		return;
	} catch (GameException e) {
		Debug::error_msg(std::string("Uncaught exception during run: ") + e.get_msg());
		return;
	}
}
