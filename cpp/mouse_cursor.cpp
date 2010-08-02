/*
mouse_cursor.cpp: Implementation of the MouseCursor class
MouseCursor is responsible for showing, hiding, and moving the mouse cursor.

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

#include "mouse_cursor.h"

#include "display.h"
#include "geometry.h"

MouseCursor::MouseCursor() : _cursor_img(GLOOTexture::load("cursor.png")), _visible(false) {}

void MouseCursor::set_visibility(bool v) {
  if (v && !_visible) { reset_pos(); }
  _visible = v;
}

void MouseCursor::handle_motion_event(const SDL_Event* event) {
  if (_visible) {
    // We often get a garbage motion event right after the program starts
    if (SDL_GetTicks() < 500) {
      return;
    }
    
    _pos += Vector(event->motion.xrel, event->motion.yrel);
          
    if (_pos.x < 0) {
      _pos.x = 0;
    } else if (_pos.x >= Display::get_screen_width()) {
      _pos.x = Display::get_screen_width()-1;
    }
    
    if (_pos.y < 0) {
      _pos.y = 0;
    } else if (_pos.y >= Display::get_screen_height()) {
      _pos.y = Display::get_screen_height()-1;
    }
  }
}

void MouseCursor::reset_pos() {
  _pos = Display::get_screen_size()/2;
}

void MouseCursor::draw() {
  if (_visible) {
    _cursor_img->draw_2d(Point(_pos - _cursor_img->get_size()/2));
  }
}