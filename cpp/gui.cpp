/*
gui.cpp: Implementation for GUI related classes and functions.

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

#include "gui.h"

#include <boost/foreach.hpp>
#include <algorithm>
#include <cmath>

#include "display.h"
#include "input.h"
#include "font.h"
#include "globals.h"
#include "gloo.h"
#include "mouse_cursor.h"

namespace GUI {
  
static const float FRAME_COLOR[4] = { 0.0, 0.0, 0.0, 0.5 };
static const float PASSIVE_COLOR[4] = { 0.5, 0.5, 0.8, 0.8 };
static const float FOCUSED_COLOR[4] = { 0.6, 0.6, 0.9, 1.0 };
static const float CHECKED_COLOR[4] = { 0.2, 0.6, 0.0, 1.0 };
static const float INNER_CHECKED_COLOR[4] = { 0.4, 0.9, 0.1, 1.0 };

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
  glColor3f(1.0, 1.0, 1.0);
  
  glPopClientAttrib();
  glEnable(GL_TEXTURE_2D);
}

void draw_diamond_box(const Box& box, const float* color) {
  draw_diamond_box(box, color[0], color[1], color[2], color[3]);
}

void Widget::emit_event(UIEvent e) {
  SDL_Event event;
  event.type = SDL_USEREVENT;
  event.user.code = e;
  event.user.data1 = (void*)(this);
  SDL_PushEvent(&event);
}

void Button::draw(const Box& box) {
  const float* color = focused() ? FOCUSED_COLOR : PASSIVE_COLOR;
  draw_diamond_box(box, color);

  int font_height = box.size.y - DIAMOND_BOX_BORDER.y*2;
  int text_width = Globals::sys_font->get_width(font_height, _label);
  Globals::sys_font->draw(box.top_left + (box.size.x - text_width)/2, font_height, _label);
}

void Button::process(const Box& box) {
  if (focused()) {
    if (Input::get_button_ch(ORSave::ButtonBoundAction::Confirm).matches_frame_events()) {
      emit_event(WIDGET_CLICKED);
    }
  }
}

void Checkbox::draw(const Box& box) {
  Box checkbox_area = box;
  checkbox_area.size.x = checkbox_area.size.y;
  draw_diamond_box(checkbox_area, focused() ? FOCUSED_COLOR : PASSIVE_COLOR);
  
  if (_value) {
    Box check_area = checkbox_area;

    for (int i = 0; i < 2; ++i) {
      check_area *= 0.8;
      draw_diamond_box(check_area, i == 0 ? CHECKED_COLOR : INNER_CHECKED_COLOR );
    }
  }
  
  int font_height = box.size.y - DIAMOND_BOX_BORDER.y*2;
  Globals::sys_font->draw(box.top_left + checkbox_area.size.x*1.2, font_height, _label);
}

void Checkbox::process(const Box& box) {
  if (focused()) {
    if (Input::get_button_ch(ORSave::ButtonBoundAction::Confirm).matches_frame_events()) {
      _value = !_value;
      emit_event(WIDGET_CLICKED);
      emit_event(WIDGET_VALUE_CHANGED);
    }
  }
}

void Slider::draw(const Box& box) {
}

void Slider::process(const Box& box) {
}

void Menu::_set_focus(WidgetList::iterator new_focus) {
  if (_focus == new_focus) {
    return;
  }

  if (_focus != _widgets.end()) {
    (*_focus)->lost_focus();
  }
  _focus = new_focus;
  if (_focus != _widgets.end()) {
    (*_focus)->got_focus();
  }
}

Menu::WidgetRegionMap& Menu::_get_regions() {
  if (_widget_regions_dirty) {
    _widget_regions.clear();
    int full_height = _widgets.size() * (_widget_height + _padding) - _padding; // Subtract one padding for fencepost error
    Point pos((Display::get_screen_width() - _width)/2, (Display::get_screen_height() - full_height)/2);
    pos += _center_offset * Display::get_screen_size();
    for (WidgetList::iterator i = _widgets.begin(); i != _widgets.end(); ++i) {
      _widget_regions[i] = Box(pos, Size(_width, _widget_height));
      pos.y += _widget_height + _padding;
    }
    _widget_regions_dirty = false;
  }

  return _widget_regions;
}

Box Menu::coverage() {
  WidgetRegionMap& regions = _get_regions();
  WidgetRegionMap::iterator top_region, bottom_region;
  top_region = regions.end();
  bottom_region = regions.end();
  for (WidgetRegionMap::iterator i = regions.begin(); i != regions.end(); ++i) {
    if (top_region == regions.end() || i->second.top_left.y < top_region->second.top_left.y) {
      top_region = i;
    }
    if (bottom_region == regions.end() || i->second.top_left.y > bottom_region->second.top_left.y) {
      bottom_region = i;
    }
  }

  return Box(
    top_region->second.top_left,
    Size(_width, bottom_region->second.top_left.y + bottom_region->second.size.y - top_region->second.top_left.y)
  );
}

void Menu::add_widget(const boost::shared_ptr<Widget>& widget) {
  _widgets.push_back(widget);

  if (_widgets.size() == 1) {
    // If this is the first widget, it gets default focus
    _set_focus(_widgets.begin());
  }

  _widget_regions_dirty = true;
}

void Menu::process() {
  WidgetRegionMap& regions = _get_regions();
  for (WidgetRegionMap::iterator i = regions.begin(); i != regions.end(); ++i) {
    (*(i->first))->process(i->second);
  }
  
  if (Globals::mouse_cursor->get_visibility()) {
    // If the mouse cursor is visible, put focus where the cursor is
    bool found_match = false;
    for (WidgetRegionMap::iterator i = regions.begin(); i != regions.end(); ++i) {
      if (i->second.contains_point(Globals::mouse_cursor->get_pos())) {
        if ((*(i->first))->focusable()) {
          found_match = true;
          _set_focus(i->first);
          break;
        }
      }
    }
    if (!found_match) {
      _set_focus(_widgets.end());
    }
  } else {
    // If the mouse cursor isn't visible, check for UI axis events to change focus
    const Channel& y_axis = Input::get_axis_ch(ORSave::AxisBoundAction::UIY);
    if (y_axis.matches_frame_events()) {
      WidgetList::iterator new_focus;
      if (_focus != _widgets.end()) {
        new_focus = _focus;
        do {
          if (y_axis.get_value() > 0.0) {
            ++new_focus;
            if (new_focus == _widgets.end()) {
              new_focus = _widgets.begin();
            }
          } else {
            if (new_focus == _widgets.begin()) {
              new_focus = _widgets.end();
            }
            --new_focus;
          }
        } while (!(*new_focus)->focusable());
      } else {
        // Nothing is currently in focus, so on input just put focus on the first widget
        new_focus = _widgets.begin();
      }
      _set_focus(new_focus);
    }
  }
}

void Menu::draw(bool frame) {
  if (frame) {
    draw_diamond_box(coverage()*1.2, FRAME_COLOR);
  }

  WidgetRegionMap& regions = _get_regions();
  for (WidgetRegionMap::iterator i = regions.begin(); i != regions.end(); ++i) {
    (*(i->first))->draw(i->second);
  }
}

}
