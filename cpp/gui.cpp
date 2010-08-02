/*
gui.cpp: Implementation for GUI related classes and functions.

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

#include "gui.h"

#include <boost/foreach.hpp>

#include "display.h"
#include "input.h"
#include "globals.h"
#include "gloo.h"
#include "mouse_cursor.h"

namespace GUI {

void draw_diamond_box(const Box& box, float r, float g, float b, float a) {
  const static GLushort indices[12] = {
    0, 1, 2,
    0, 2, 5,
    5, 3, 0,
    5, 4, 3
  };
  
  GLfloat points[12] = {
    box.top_left.x + DIAMOND_BOX_BORDER.x/2, box.top_left.y,
    box.top_left.x, box.top_left.y + box.size.y/2,
    box.top_left.x + DIAMOND_BOX_BORDER.x/2, box.top_left.y + box.size.y,
    box.top_left.x + box.size.x - DIAMOND_BOX_BORDER.x/2, box.top_left.y,
    box.top_left.x + box.size.x, box.top_left.y + box.size.y/2,
    box.top_left.x + box.size.x - DIAMOND_BOX_BORDER.x/2, box.top_left.y + box.size.y
  };
  
  glDisable(GL_TEXTURE_2D);
  glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  
  glColor4f(r, g, b, a);
  glVertexPointer(2, GL_FLOAT, 0, points);
  glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, indices);
  
  glPopClientAttrib();
  glEnable(GL_TEXTURE_2D);
}

void FocusTracker::add_region(const std::string& name, const Box& box) {
  _focus_regions[name] = box;
  
  // If this is the first region being added, make it the default focus
  if (_focus_regions.size() == 1) {
    _focus_iter = _focus_regions.begin();
  }
}

const Box* FocusTracker::get_region(const std::string& name) const {
  std::map<std::string, Box>::const_iterator i = _focus_regions.find(name);
  if (i == _focus_regions.end()) {
    return NULL;
  } else {
    return &(i->second);
  }
}

void FocusTracker::process() {
  // TODO: Check for bound UI events (for keyboard focus)
  
  // Check for mouse motion events, put focus where the mouse cursor is
  BOOST_FOREACH(const SDL_Event& event, Globals::frame_events) {
    if (event.type == SDL_MOUSEMOTION) {
      _focus_mode = MOUSE_FOCUS;
      
      bool found_match = false;
      for(std::map<std::string, Box>::const_iterator i = _focus_regions.begin(); i != _focus_regions.end(); ++i) {
        if (i->second.contains_point(Globals::mouse_cursor->get_pos())) {
          found_match = true;
          _focus_iter = i;
          break;
        }
      }
      if (!found_match) { _focus_iter = _focus_regions.end(); }
    }
  }
}

std::string FocusTracker::get_current_focus() const {
  if (_focus_iter == _focus_regions.end()) {
    return std::string("");
  } else {
    return _focus_iter->first;
  }
}

void SimpleMenu::add_button(const std::string& name, const std::string& label) {
  _entries.push_back(std::pair<std::string, std::string>(name, label));
}

void SimpleMenu::process() {
  if (!_filled_focus_tracker) {
    int full_height = _entries.size() * (_btn_height + _padding) - _padding; // Subtract one padding for fencepost error
    Point pos((Display::get_screen_width() - _width)/2, (Display::get_screen_height() - full_height)/2);
    for (std::list<std::pair<std::string, std::string> >::const_iterator i = _entries.begin(); i != _entries.end(); ++i) {
      _focus_tracker.add_region(i->first, Box(pos, Size(_width, _btn_height)));
      pos.y += _btn_height + _padding;
    }
    
    _filled_focus_tracker = true;
  }
  
  _focus_tracker.process();
  
  // Check for events that would activate the currently focused entry
}

void SimpleMenu::draw() {
  std::string cur_activated = get_activated_button();
  std::string cur_focus = _focus_tracker.get_current_focus();
  
  for (std::list<std::pair<std::string, std::string> >::const_iterator i = _entries.begin(); i != _entries.end(); ++i) {
    const Box* region = _focus_tracker.get_region(i->first);
    if (cur_activated == i->first) {
      GUI::draw_diamond_box(
        *region,
        BUTTON_ACTIVATED_COLOR[0],
        BUTTON_ACTIVATED_COLOR[1],
        BUTTON_ACTIVATED_COLOR[2],
        BUTTON_ACTIVATED_COLOR[3]
      );
    } else if (cur_focus == i->first) {
      GUI::draw_diamond_box(
        *region,
        BUTTON_FOCUSED_COLOR[0],
        BUTTON_FOCUSED_COLOR[1],
        BUTTON_FOCUSED_COLOR[2],
        BUTTON_FOCUSED_COLOR[3]
      );
    } else {
      GUI::draw_diamond_box(
        *region,
        BUTTON_PASSIVE_COLOR[0],
        BUTTON_PASSIVE_COLOR[1],
        BUTTON_PASSIVE_COLOR[2],
        BUTTON_PASSIVE_COLOR[3]
      );
    }
    
    glColor3f(1.0, 1.0, 1.0);
    int font_height = _btn_height - DIAMOND_BOX_BORDER.y*2;
    int text_width = Globals::sys_font->get_width(font_height, i->second);
    Globals::sys_font->draw(region->top_left + (_width - text_width)/2, font_height, i->second);
  }
}

std::string SimpleMenu::get_activated_button() {
  return "";
}

}