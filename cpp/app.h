#ifndef ORBIT_RIBBON_APP_H
#define ORBIT_RIBBON_APP_H

#include <GL/gl.h>

#include <string>
#include <list>

union SDL_Event;

class App {
	public:
		static void run(const std::list<std::string>& arguments);
		
		static const std::list<SDL_Event>& get_frame_events();
		
		static GLint get_total_steps();
		
		static GLint get_max_fps() { return 60; }
		static std::string get_version() { return std::string("prealpha"); }
	
	private:
		static void frame_loop();
};

#endif
