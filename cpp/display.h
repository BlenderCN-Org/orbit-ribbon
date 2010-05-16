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
	private:
		static int screen_width, screen_height;
		static GLfloat screen_ratio;
	
	public:
		static int get_screen_width() { return screen_width; }
		static int get_screen_height() { return screen_height; }
		static float get_screen_ratio() { return screen_ratio; }
		
	private:
		static void init();
		
		static void draw_frame();
		static void screen_resize();
		
		friend class App;
};

#endif
