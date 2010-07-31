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
#include "gloo.h"

bool Widget::bbox_contains_point(const Point& upper_left, const Point& test_pt) const {
  Size bbox_size = get_bbox_size();
  return (
    test_pt.x >= upper_left.x &&
    test_pt.y >= upper_left.y &&
    test_pt.x < upper_left.x + bbox_size.x &&
    test_pt.y < upper_left.y + bbox_size.y
  );
}

void BoxWidget::draw(const Point& upper_left, Widget::DrawMode mode) const {
}

bool BoxWidget::widget_contains_point(const Point& test_pt) const {
}

Size BoxWidget::get_bbox_size() const {
}

std::list<WidgetLocation> BoxWidget::get_focusable_children(const Point& upper_left) const {
}

ButtonWidget::ButtonWidget(const boost::shared_ptr<Widget>& child) : BoxWidget(child) {
}

void ButtonWidget::draw(const Point& upper_left, Widget::DrawMode mode) const {
}

void LayoutWidget::add_child(const boost::shared_ptr<Widget>& widget) {
}

void LayoutWidget::draw(const Point& upper_left, Widget::DrawMode mode) const {
}

Size LayoutWidget::get_bbox_size() const {
}

std::list<WidgetLocation> LayoutWidget::get_focusable_children(const Point& upper_left) const {
}

void TextWidget::draw(const Point& upper_left, Widget::DrawMode mode) const {
}

Size TextWidget::get_bbox_size() const {
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
