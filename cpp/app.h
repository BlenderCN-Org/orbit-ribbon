#ifndef ORBIT_RIBBON_APP_H
#define ORBIT_RIBBON_APP_H

#include <ode/ode.h>

#include <string>

struct SDL_Surface;

#define ORBIT_RIBBON_VERSION "prealpha"

// FIXME Ugly and inelegant and hard-coded
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SCREEN_BPP 16
#define MAX_FPS 60

class App {
	public:
		static void init();
		static void load_area(const std::string& area_name);
		static void load_mission(const std::string& mission_name);
		
		static dWorldID get_ode_world() { return _ode_world; }
		static dSpaceID get_static_space() { return _static_space; }
		static dSpaceID get_dyn_space() { return _dyn_space; }
		
		static void run();
		
	private:
		static SDL_Surface* _screen;
		
		static dWorldID _ode_world;
		static dSpaceID _static_space;
		static dSpaceID _dyn_space;
		static dJointGroupID _contact_group;
		
		static void _sim_step();
		static void _draw_frame();
};

#endif
