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

#include <boost/lexical_cast.hpp>
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
  GLfloat points[12] = {
    box.top_left.x + DIAMOND_BOX_BORDER.x/2, box.top_left.y,
    box.top_left.x, box.top_left.y + box.size.y/2,
    box.top_left.x + DIAMOND_BOX_BORDER.x/2, box.top_left.y + box.size.y,
    box.top_left.x + box.size.x - DIAMOND_BOX_BORDER.x/2, box.top_left.y,
    box.top_left.x + box.size.x, box.top_left.y + box.size.y/2,
    box.top_left.x + box.size.x - DIAMOND_BOX_BORDER.x/2, box.top_left.y + box.size.y
  };
  
  const static GLushort indices[12] = {
    0, 1, 2,
    0, 2, 5,
    5, 3, 0,
    5, 4, 3
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

void draw_box(const Box& box, float r, float g, float b, float a) {
  GLfloat points[8] = {
    box.top_left.x, box.top_left.y,
    box.top_left.x, box.top_left.y+box.size.y,
    box.top_left.x+box.size.x, box.top_left.y+box.size.y,
    box.top_left.x+box.size.x, box.top_left.y,
  };

  glDisable(GL_TEXTURE_2D);
  glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  glColor4f(r, g, b, a);
  glVertexPointer(2, GL_FLOAT, 0, points);
  glDrawArrays(GL_QUADS, 0, 8);
  glColor3f(1.0, 1.0, 1.0);

  glPopClientAttrib();
  glEnable(GL_TEXTURE_2D);
}

void draw_box(const Box& box, const float* color) {
  draw_box(box, color[0], color[1], color[2], color[3]);
}

void Widget::emit_event(UIEvent e) {
  SDL_Event event;
  event.type = SDL_USEREVENT;
  event.user.code = e;
  event.user.data1 = (void*)(this);
  SDL_PushEvent(&event);
}

void Label::draw(const Box& box) {
  int font_height = box.size.y - DIAMOND_BOX_BORDER.y*2;
  int text_width = Globals::sys_font->get_width(font_height, _label);
  Globals::sys_font->draw(box.top_left + (box.size.x - text_width)*_x_align, font_height, _label);
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

const float SLIDER_GAUGE_SIZE_COEF = 0.7;
const float SLIDER_PUSH_CHANGE = 0.05;

Box Slider::_gauge_area(const Box& box) {
  Box r = box;
  r.size.x = r.size.y*5;
  r *= SLIDER_GAUGE_SIZE_COEF;
  return r;
}

void Slider::draw(const Box& box) {
  Box gauge_area = _gauge_area(box);
  Box gauge_dbox_area = gauge_area/SLIDER_GAUGE_SIZE_COEF;
  gauge_dbox_area.size.x = gauge_dbox_area.size.y*5;
  draw_diamond_box(gauge_dbox_area, focused() ? FOCUSED_COLOR : PASSIVE_COLOR);

  GLfloat gauge_points[10] = {
    gauge_area.top_left.x, gauge_area.top_left.y + gauge_area.size.y, // Point 0 : Left tip of triangle
    gauge_area.top_left.x + _value*gauge_area.size.x, gauge_area.top_left.y + gauge_area.size.y, // Point 1 : Bottom of gauge line
    gauge_area.top_left.x + _value*gauge_area.size.x, gauge_area.top_left.y + (1-_value)*gauge_area.size.y, // Point 2 : Top of gauge line
    gauge_area.top_left.x + gauge_area.size.x, gauge_area.top_left.y, // Point 3 : Upper right corner
    gauge_area.top_left.x + gauge_area.size.x, gauge_area.top_left.y + gauge_area.size.y // Point 4 : Lower right corner
  };

  glDisable(GL_TEXTURE_2D);
  glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, gauge_points);

  const static GLushort triangle_indices[3] = { 0, 1, 2 };
  GLfloat triangle_colors[9] = {
    0.5, 0.5, 0.5, // Color 0 : Left tip of triangle
    0.3, 0.3+(0.6*_value), 0.3, // Color 1 : Bottom of gauge line
    0.3, 0.3+(0.6*_value), 0.3 // Color 2 : Top of gauge line
  };
  glColorPointer(3, GL_FLOAT, 0, triangle_colors);
  glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, triangle_indices);

  const static GLushort quad_indices[4] = { 4, 3, 2, 1 };
  GLfloat quad_colors[15] = { // TODO: Maybe make this area a pretty gradient too
    0.0, 0.0, 0.0,
    0.1, 0.1, 0.1,
    0.1, 0.1, 0.1,
    0.1, 0.1, 0.1,
    0.1, 0.1, 0.1
  };
  glColorPointer(3, GL_FLOAT, 0, quad_colors);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_SHORT, quad_indices);

  glPopClientAttrib();
  glEnable(GL_TEXTURE_2D);

  int font_height = box.size.y - DIAMOND_BOX_BORDER.y*2;
  Globals::sys_font->draw(gauge_dbox_area.top_left + gauge_dbox_area.size.x*1.1, font_height, _label);

  int n_font_height = gauge_dbox_area.size.y*0.8;
  Globals::sys_font->draw(gauge_dbox_area.top_left + gauge_dbox_area.size * 0.05, n_font_height, boost::lexical_cast<std::string>(std::floor(_value*100 + 0.5)));
}

void Slider::process(const Box& box) {
  if (focused()) {
    bool changed = false;
    Box gauge_area = _gauge_area(box);
    Box gauge_sense_area = gauge_area * 1.2;
    Point mouse = Globals::mouse_cursor->get_pos();

    if (Input::get_axis_ch(ORSave::AxisBoundAction::UIX).matches_frame_events()) {
      const Channel& x_axis = Input::get_axis_ch(ORSave::AxisBoundAction::UIX);
      if (x_axis.get_value() > 0.0) {
        _value += SLIDER_PUSH_CHANGE;
      } else {
        _value -= SLIDER_PUSH_CHANGE;
      }
      changed = true;
    } else if (
      Globals::mouse_cursor->get_visibility() &&
      Input::get_button_ch(ORSave::ButtonBoundAction::Confirm).is_on() &&
      gauge_sense_area.contains_point(mouse)
    ) {
      _value = (mouse.x - gauge_area.top_left.x)/gauge_area.size.x;
      changed = true;
    }

    if (changed) {
      if (_value > 1.0) { _value = 1.0; }
      else if (_value < 0.0) { _value = 0.0; }
      emit_event(WIDGET_VALUE_CHANGED);
    }
  }
}

void WidgetLayout::clear_region_map_cache() {
  _widget_regions_dirty = true;
  _coverage_dirty = true;
}

const WidgetLayout::RegionMap& WidgetLayout::get_regions() {
  if (_widget_regions_dirty) {
    _cached_widget_regions.clear();
    populate_widget_region_map(_cached_widget_regions);
    _widget_regions_dirty = false;
  }

  return _cached_widget_regions;
}

void WidgetLayout::set_focus(Widget* new_focus) {
  if (_focus == new_focus) {
    return;
  }

  if (_focus != NULL) {
    _focus->lost_focus();
  }
  _focus = new_focus;
  if (_focus != NULL) {
    _focus->got_focus();
  }
}

Box WidgetLayout::coverage() {
  if (_coverage_dirty) {
    const RegionMap& regions = get_regions();
    RegionMap::const_iterator top_region, bottom_region, left_region, right_region;
    top_region = regions.end();
    bottom_region = regions.end();
    left_region = regions.end();
    right_region = regions.end();

    for (RegionMap::const_iterator i = regions.begin(); i != regions.end(); ++i) {
      if (top_region == regions.end() || i->second.top_left.y < top_region->second.top_left.y) { top_region = i; }
      if (bottom_region == regions.end() || i->second.top_left.y > bottom_region->second.top_left.y) { bottom_region = i; }
      if (left_region == regions.end() || i->second.top_left.x < left_region->second.top_left.x) { left_region = i; }
      if (right_region == regions.end() || i->second.top_left.x > right_region->second.top_left.x) { right_region = i; }
    }

    _coverage = Box(
      Point(left_region->second.top_left.x, top_region->second.top_left.y),
      Size(
        right_region->second.top_left.x + right_region->second.size.x - left_region->second.top_left.x,
        bottom_region->second.top_left.y + bottom_region->second.size.y - top_region->second.top_left.y
      )
    );

    _coverage_dirty = false;
  }

  return _coverage;
}

void WidgetLayout::draw(bool frame) {
  if (frame) {
    draw_diamond_box(coverage()*1.2, FRAME_COLOR);
  }

  const RegionMap& regions = get_regions();
  for (RegionMap::const_iterator i = regions.begin(); i != regions.end(); ++i) {
    i->first->draw(i->second);
  }
}

void WidgetLayout::process() {
  const RegionMap& regions = get_regions();
  for (RegionMap::const_iterator i = regions.begin(); i != regions.end(); ++i) {
    i->first->process(i->second);
  }
  
  if (Globals::mouse_cursor->get_visibility()) {
    // If the mouse cursor is visible, put focus where the cursor is
    bool found_match = false;
    for (RegionMap::const_iterator i = regions.begin(); i != regions.end(); ++i) {
      if (i->second.contains_point(Globals::mouse_cursor->get_pos())) {
        if (i->first->focusable()) {
          found_match = true;
          set_focus(i->first);
          break;
        }
      }
    }
    if (!found_match) {
      set_focus(NULL);
    }
  }
}

void Menu::populate_widget_region_map(WidgetLayout::RegionMap& m) {
  int full_height = _widgets.size() * (_widget_height + _padding) - _padding; // Subtract one padding for fencepost error
  Point pos((Display::get_screen_width() - _width)/2, (Display::get_screen_height() - full_height)/2);
  pos += _center_offset * Display::get_screen_size();
  for (WidgetList::iterator i = _widgets.begin(); i != _widgets.end(); ++i) {
    m[i->get()] = Box(pos, Size(_width, _widget_height));
    pos.y += _widget_height + _padding;
  }
}

void Menu::add_widget(const boost::shared_ptr<Widget>& widget) {
  _widgets.push_back(widget);
  clear_region_map_cache();
  if (!get_focus() && widget->focusable()) { set_focus(&*widget); }
}

void Menu::process() {
  WidgetLayout::process();

  if (!Globals::mouse_cursor->get_visibility()) {
    // If the mouse cursor isn't visible, check for UI axis events to change focus
    const Channel& y_axis = Input::get_axis_ch(ORSave::AxisBoundAction::UIY);
    if (y_axis.matches_frame_events()) {
      WidgetList::iterator focus_iter;
      for (focus_iter = _widgets.begin(); focus_iter != _widgets.end(); ++focus_iter) {
        if (focus_iter->get() == get_focus()) {
          break;
        }
      }

      if (focus_iter != _widgets.end()) {
        // Move the focus along the menu in the player's intended direction
        do {
          if (y_axis.get_value() > 0.0) {
            ++focus_iter;
            if (focus_iter == _widgets.end()) {
              focus_iter = _widgets.begin();
            }
          } else {
            if (focus_iter == _widgets.begin()) {
              focus_iter = _widgets.end();
            }
            --focus_iter;
          }
        } while (!(*focus_iter)->focusable());

        set_focus(focus_iter->get());
      } else {
        // Nothing is currently in focus, so just put focus on the first widget
        set_focus(_widgets.front().get());
      }
    }
  }
}

void Grid::populate_widget_region_map(WidgetLayout::RegionMap& m) {
  int full_height = _rows.size() * (_row_height + _padding) - _padding; // Subtract one padding for fencepost error
  Point row_pos((Display::get_screen_width() - _width)/2, (Display::get_screen_height() - full_height)/2);
  BOOST_FOREACH(const Row& row, _rows) {
    Point cell_pos(row_pos);
    if (row.cells.size() > 0) {
      int cell_width = (_width - (row.cells.size()-1)*_padding)/row.cells.size();
      BOOST_FOREACH(const boost::shared_ptr<Widget>& cell, row.cells) {
        m[cell.get()] = Box(cell_pos, Size(cell_width, _row_height));
        cell_pos.x += cell_width + _padding;
      }
    }
    row_pos.y += _row_height + _padding;
  }
}

void Grid::add_row(bool force_left_focus) {
  _rows.push_back(Row(force_left_focus));
  clear_region_map_cache();
}

void Grid::add_cell(const boost::shared_ptr<Widget>& widget) {
  _rows.back().cells.push_back(widget);
  clear_region_map_cache();
  if (!get_focus() && widget->focusable()) { set_focus(&*widget); }
}

void Grid::process() {
  WidgetLayout::process();

  if (!Globals::mouse_cursor->get_visibility()) {
    // If the mouse cursor isn't visible, check for UI axis events to change focus
    const Channel& x_axis = Input::get_axis_ch(ORSave::AxisBoundAction::UIX);
    const Channel& y_axis = Input::get_axis_ch(ORSave::AxisBoundAction::UIY);

    bool x_moved = x_axis.matches_frame_events();
    bool y_moved = y_axis.matches_frame_events();

    if (x_moved || y_moved) {
      std::list<Row>::iterator row_iter;
      std::list<boost::shared_ptr<Widget> >::iterator cell_iter;
      unsigned int cell_index;
      for (row_iter = _rows.begin(); row_iter != _rows.end(); ++row_iter) {
        cell_index = 0;
        for (cell_iter = row_iter->cells.begin(); cell_iter != row_iter->cells.end(); ++cell_iter) {
          if (cell_iter->get() == get_focus()) {
            break;
          }
          ++cell_index;
        }
        if (cell_iter != row_iter->cells.end()) {
          break;
        }
      }

      if (row_iter != _rows.end()) {
        if (x_moved) {
          // Move horizontally among cells in the same row
          do {
            if (x_axis.get_value() > 0.0) {
              ++cell_iter;
              if (cell_iter == row_iter->cells.end()) {
                cell_iter = row_iter->cells.begin();
              }
            } else {
              if (cell_iter == row_iter->cells.begin()) {
                cell_iter = row_iter->cells.end();
              }
              --cell_iter;
            }
          } while (!(*cell_iter)->focusable());

          set_focus(cell_iter->get());
        } else if (y_moved) {
          // Move vertically among rows
          int row_valid_cells;
          do {
            if (y_axis.get_value() > 0.0) {
              ++row_iter;
              if (row_iter == _rows.end()) {
                row_iter = _rows.begin();
              }
            } else {
              if (row_iter == _rows.begin()) {
                row_iter = _rows.end();
              }
              --row_iter;
            }

            row_valid_cells = 0;
            for (cell_iter = row_iter->cells.begin(); cell_iter != row_iter->cells.end(); ++cell_iter) {
              if ((*cell_iter)->focusable()) {
                ++row_valid_cells;
              }
            }
          } while (row_valid_cells == 0);

          // Pick a cell in the new row to focus
          cell_iter = row_iter->cells.begin();
          if (!row_iter->force_left_focus) {
            for (unsigned int i = 0; i < cell_index; ++i) {
              ++cell_iter;
              if (cell_iter == row_iter->cells.end()) {
                --cell_iter;
                break;
              }
            }
          }
          while (!(*cell_iter)->focusable() && cell_iter != row_iter->cells.begin()) {
            --cell_iter;
          }
          while (!(*cell_iter)->focusable() && cell_iter != row_iter->cells.end()) {
            ++cell_iter;
          }
          set_focus(cell_iter->get());
        }
      } else {
        // Nothing is currently in focus, so just put focus on the first focusable cell we can find 
        bool done = false;
        for (std::list<Row>::iterator row = _rows.begin(); row != _rows.end(); ++row) {
          for (std::list<boost::shared_ptr<Widget> >::iterator cell = row->cells.begin(); cell != row->cells.end(); ++cell) {
            if ((*cell)->focusable()) {
              set_focus(cell->get());
              done = true;
              break;
            }
          }
          if (done) {
            break;
          }
        }
      }
    }
  }
}

}
