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

#include "globals.h"
#include "gloo.h"

bool bbox_contains_point(const Size& bbox_size, const Point& upper_left, const Point& test_pt){
  return (
    test_pt.x >= upper_left.x &&
    test_pt.y >= upper_left.y &&
    test_pt.x < upper_left.x + bbox_size.x &&
    test_pt.y < upper_left.y + bbox_size.y
  );
}

Size WidgetLocation::get_bbox_size() {
  return widget->get_bbox_size();
}

void BoxWidget::draw(const Point& upper_left, float r, float g, float b, float a) const {
    GUI::draw_box(upper_left, get_bbox_size(), r, g, b, a);
}

Size BoxWidget::get_bbox_size() const {
  return _child->get_bbox_size() + (GUI_BOX_BORDER*2);
}

std::list<WidgetLocation> BoxWidget::get_children_locations(const Point& upper_left) const {
  std::list<WidgetLocation> ret;
  ret.push_back(WidgetLocation(&*_child, upper_left + GUI_BOX_BORDER));
  return ret;
}

const float ButtonWidget::PASSIVE_COLOR[4] = { 0.5, 0.5, 0.8, 0.8 };
const float ButtonWidget::FOCUSED_COLOR[4] = { 0.7, 0.7, 1.0, 1.0 };
const float ButtonWidget::ACTIVATED_COLOR[4] = { 0.5, 0.8, 0.5, 1.0 };

void ButtonWidget::draw(const Point& upper_left, Widget::DrawMode mode) const {
  switch(mode) {
    case WIDGET_PASSIVE:
      BoxWidget::draw(upper_left, PASSIVE_COLOR[0], PASSIVE_COLOR[1], PASSIVE_COLOR[2], PASSIVE_COLOR[3]);
      break;
    case WIDGET_FOCUSED:
      BoxWidget::draw(upper_left, FOCUSED_COLOR[0], FOCUSED_COLOR[1], FOCUSED_COLOR[2], FOCUSED_COLOR[3]);
      break;
    case WIDGET_ACTIVATED:
      BoxWidget::draw(upper_left, ACTIVATED_COLOR[0], ACTIVATED_COLOR[1], ACTIVATED_COLOR[2], ACTIVATED_COLOR[3]);
      break;
  }
}

void LayoutWidget::add_child(const boost::shared_ptr<Widget>& widget) {
  _children.push_back(widget);
}

void LayoutWidget::draw(const Point& upper_left, Widget::DrawMode mode) const {
  Point pos(upper_left);
  for (std::list<boost::shared_ptr<Widget> >::const_iterator i = _children.begin(); i != _children.end(); ++i) {
    const boost::shared_ptr<Widget>& widget = *i;
    widget->draw(pos, mode);
    switch(_orientation) {
      case WIDGET_HORIZONTAL:
        pos.x += widget->get_bbox_size().x + LAYOUT_PADDING;
        break;
      case WIDGET_VERTICAL:
        pos.y += widget->get_bbox_size().y + LAYOUT_PADDING;
        break;
    }
  }
}

Size LayoutWidget::get_bbox_size() const {
  Size ret(0, 0);
  
  for (std::list<boost::shared_ptr<Widget> >::const_iterator i = _children.begin(); i != _children.end(); ++i) {
    const boost::shared_ptr<Widget>& widget = *i;
    Size widget_size = widget->get_bbox_size();
    switch(_orientation) {
      case WIDGET_HORIZONTAL:
        ret.x += widget_size.x + LAYOUT_PADDING;
        if (widget_size.y > ret.y) {
          ret.y = widget_size.y;
        }
        break;
      case WIDGET_VERTICAL:
        ret.y += widget_size.y + LAYOUT_PADDING;
        if (widget_size.x > ret.x) {
          ret.x = widget_size.x;
        }
        break;
    }
  }
  
  return ret;
}

std::list<WidgetLocation> LayoutWidget::get_children_locations(const Point& upper_left) const {
  std::list<WidgetLocation> ret;
  
  Point pos(upper_left);
  for (std::list<boost::shared_ptr<Widget> >::const_iterator i = _children.begin(); i != _children.end(); ++i) {
    const boost::shared_ptr<Widget>& widget = *i;
    ret.push_back(WidgetLocation(&*widget, pos));
    switch(_orientation) {
      case WIDGET_HORIZONTAL:
        pos.x += widget->get_bbox_size().x + LAYOUT_PADDING;
        break;
      case WIDGET_VERTICAL:
        pos.y += widget->get_bbox_size().y + LAYOUT_PADDING;
        break;
    }
  }
  
  return ret;
}

void TextWidget::draw(const Point& upper_left, Widget::DrawMode mode __attribute__ ((unused))) const {
  Globals::sys_font->draw(upper_left, _font_height, _msg);
}

Size TextWidget::get_bbox_size() const {
  return Size(Globals::sys_font->get_width(_font_height, _msg), _font_height);
}

void GUI::draw_box(const Point& top_left, const Size& size, float r, float g, float b, float a) {
  const static GLushort indices[12] = {
    0, 1, 2,
    0, 2, 5,
    5, 3, 0,
    5, 4, 3
  };
  
  GLfloat points[12] = {
    top_left.x + GUI_BOX_BORDER.x/2, top_left.y,
    top_left.x, top_left.y + size.y/2,
    top_left.x + GUI_BOX_BORDER.x/2, top_left.y + size.y,
    top_left.x + size.x - GUI_BOX_BORDER.x/2, top_left.y,
    top_left.x + size.x, top_left.y + size.y/2,
    top_left.x + size.x - GUI_BOX_BORDER.x/2, top_left.y + size.y
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