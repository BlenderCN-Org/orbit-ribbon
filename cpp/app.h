#ifndef ORBIT_RIBBON_APP_H
#define ORBIT_RIBBON_APP_H

#include <GL/gl.h>

#include <string>
#include <list>

union SDL_Event;

class App {
	public:
		static void init();
		static void load_area(const std::string& area_name);
		static void load_mission(const std::string& mission_name);
		
		static void run();
		
		static const std::list<SDL_Event>& get_frame_events();
		
		static void set_fade_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
		static void set_fade(bool flag);
		
		static GLsizei get_screen_width();
		static GLsizei get_screen_height();
		static GLint get_screen_depth();
		static GLint get_total_steps();
		
		static GLint get_max_fps() { return 60; }
		static std::string get_version() { return std::string("prealpha"); }
};

#endif
