#ifndef ORBIT_RIBBON_DISPLAY_H
#define ORBIT_RIBBON_DISPLAY_H

#include <GL/gl.h>

class App;

class Display {
	public:
		// FIXME : Abstract this out to the game mode controller
		static void set_fade_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
		static void set_fade(bool flag);
		
		static GLsizei get_screen_width();
		static GLsizei get_screen_height();
		static GLint get_screen_depth();
		
	private:
		static void _init();
		
		static void _draw_frame();
		static void _screen_resize();
		
		friend class App;
};

#endif
