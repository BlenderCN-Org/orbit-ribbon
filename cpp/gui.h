/*
gui.h: Header for GUI related classes and functions.

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

#ifndef ORBIT_RIBBON_GUI_H
#define ORBIT_RIBBON_GUI_H

#include <boost/shared_ptr.hpp>
#include <string>
#include <map>
#include <list>

#include "geometry.h"
#include "gloo.h"

namespace GUI {
  const Vector DIAMOND_BOX_BORDER(8, 2); // A diamond box's contents are drawn this number of pixels from the left/right and top/bottom of the box respectively
  void draw_diamond_box(const Box& box, float r = 0.0, float g = 0.0, float b = 0.0, float a = 0.5);
  
  class Widget {
    private:
      bool _focused;

    public:
      Widget() : _focused(false) {}

      virtual void got_focus() { _focused = true; }
      virtual void lost_focus() { _focused = false; }
      bool focused() { return _focused; }

      virtual void draw(const Box& box) =0;
  };

  class Button : public Widget {
    private:
      const float BUTTON_PASSIVE_COLOR[4] = { 0.5, 0.5, 0.8, 0.8 };
      const float BUTTON_FOCUSED_COLOR[4] = { 0.7, 0.7, 1.0, 1.0 };

      std::string _label;

    public:
      Button(const std::string& label) : _label(label) {}
      void draw(const Box& box);
  };

  class SimpleMenu {
    private:
      int _width, _widget_height, _padding;
      Vector _center_offset;

      typedef std::list<boost::shared_ptr<Widget> > WidgetList;
      WidgetList _widgets;
      WidgetList::iterator _focus;

    public:
      SimpleMenu(int width, int widget_height, int padding, Vector center_offset = Vector(0,0,0)) :
        _width(width), _widget_height(widget_height), _padding(padding), _center_offset(center_offset), _widgets(), _focus(_widgets.end()) {}

      void add_widget(const Widget& widget);

      void process();
      void draw();

      void reset_focus() { _focus = _widgets.end(); }
  };
}

#endif
