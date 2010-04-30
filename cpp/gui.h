/*
gui.h: Header for GUI related classes and functions.

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

#ifndef ORBIT_RIBBON_GUI_H
#define ORBIT_RIBBON_GUI_H

#include "geometry.h"
#include "gloo.h"

const Vector GUI_BOX_BORDER(8, 2); // Number of pixels from the left/right and top/bottom of a box you should draw its contents 
const GLfloat GUI_BOX_COLOR[4] = {0.0, 0.0, 1.0, 0.5}; // Color of the box

class Gui {
	public:
		static void draw_box(const Point& top_left, const Size& size);
};

#endif