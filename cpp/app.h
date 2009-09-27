#ifndef ORBIT_RIBBON_APP_H
#define ORBIT_RIBBON_APP_H

#include <string>

struct SDL_Surface;

// FIXME Ugly and inelegant and hard-coded
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SCREEN_BPP 16

class App {
	public:
		static void init();
		static void load_area(const std::string& area_name);
		static void load_mission(const std::string& mission_name);
		
		static void run();
		
	private:
		static SDL_Surface* _screen;
		
		static void _sim_step();
		static void _draw_frame();
};

#endif
