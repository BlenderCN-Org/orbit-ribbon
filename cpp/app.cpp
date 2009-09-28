#include <GL/gl.h>
#include <SDL/SDL.h>

#include <string>
#include <list>

#include "app.h"
#include "display.h"
#include "except.h"
#include "sim.h"

GLint total_steps = 0;

std::list<SDL_Event> frame_events;

void App::init() {
	Display::_init();
	Sim::_init();
}

void App::load_area(const std::string& area_name) {
}

void App::load_mission(const std::string& mission_name) {
}

void App::run() {
	const GLint max_ticks_per_frame = 1000/get_max_fps();
	int unsimulated_ticks = 0;
	
	while (1) {
		GLint frame_start = SDL_GetTicks();
		
		// Add all new events to the events list for this frame, and quit on QUIT events
		frame_events.clear();
		SDL_Event event;
	   while (SDL_PollEvent(&event)) {
			frame_events.push_back(event);
		  	if (event.type == SDL_QUIT) {
				throw GameQuitException("Closed on quit event");
			}
		}
		
		// Do simulation steps until we've caught up with the display
		while (unsimulated_ticks > max_ticks_per_frame) {
			Sim::_sim_step();
			total_steps += 1;
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

const std::list<SDL_Event>& App::get_frame_events() {
	return frame_events;
}

GLint App::get_total_steps() {
	return 16;
}
