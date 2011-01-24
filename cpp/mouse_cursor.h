/*
mouse_cursor.h: Header of the MouseCursor class
MouseCursor is responsible for showing, hiding, and moving the mouse cursor.

Copyright 2011 David Simon <david.mike.simon@gmail.com>

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

#ifndef ORBIT_RIBBON_MOUSE_CURSOR_H
#define ORBIT_RIBBON_MOUSE_CURSOR_H

#include <boost/shared_ptr.hpp>

#include "geometry.h"
#include "gloo.h"

union SDL_Event;

class MouseCursor {
  private:
    boost::shared_ptr<GLOOTexture> _cursor_img;
    Point _pos;
    bool _visible;
    bool _got_first_event;
  
  public:
    MouseCursor();
    
    void process_events();
    void reset_pos();
    Point get_pos() const { return _pos; }
    bool get_visibility() const { return _visible; }
    void set_visibility(bool v) { _visible = v; }
    
    void draw();
};

#endif
