/*
display.h: Header of the Display class
Display is responsible for interacting with SDL's video capabilities and for drawing each frame.

Copyright 2009 David Simon. You can reach me at david.mike.simon@gmail.com

This file is part of Orbit Ribbon.

Orbit Ribbon is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Orbit Ribbon is distributed in the hope that it will be awesome,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Orbit Ribbon.  If not, see http://www.gnu.org/licenses/
*/

#ifndef ORBIT_RIBBON_DISPLAY_H
#define ORBIT_RIBBON_DISPLAY_H

#include <GL/glew.h>
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
