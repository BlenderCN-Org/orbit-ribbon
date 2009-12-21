#ifndef ORBIT_RIBBON_APP_H
#define ORBIT_RIBBON_APP_H

#include <GL/gl.h>

#include <string>
#include <vector>

union SDL_Event;

class App {
	public:
		static void run(const std::vector<std::string>& arguments);
		
		static const std::vector<SDL_Event>& get_frame_events();
		
		static GLint get_total_steps();
		
	private:
		static void frame_loop();
};

#endif
