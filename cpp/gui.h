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
  const float BUTTON_PASSIVE_COLOR[4] = { 0.5, 0.5, 0.8, 0.8 };
  const float BUTTON_FOCUSED_COLOR[4] = { 0.7, 0.7, 1.0, 1.0 };
  const Vector DIAMOND_BOX_BORDER(8, 2); // A diamond box's contents are drawn this number of pixels from the left/right and top/bottom of the box respectively
  
  void draw_diamond_box(const Box& box, float r = 0.0, float g = 0.0, float b = 0.0, float a = 0.5);

  enum UIEvent { WIDGET_GOT_FOCUS, WIDGET_LOST_FOCUS, WIDGET_CLICKED };
  
  class Widget {
    private:
      bool _focused;

    protected:
      void emit_event(UIEvent e);

    public:
      Widget() : _focused(false) {}

      virtual void got_focus() { _focused = true; emit_event(WIDGET_GOT_FOCUS); }
      virtual void lost_focus() { _focused = false; emit_event(WIDGET_LOST_FOCUS); }
      bool focused() { return _focused; }

      virtual void draw(const Box& box) =0;
      virtual void process(const Box& box) =0;
  };

  class Button : public Widget {
    private:
      std::string _label;

    public:
      Button(const std::string& label) : _label(label) {}
      void draw(const Box& box);
      void process(const Box& box);
  };

  class Menu {
    private:
      int _width, _widget_height, _padding;
      Vector _center_offset;

      typedef std::list<boost::shared_ptr<Widget> > WidgetList;
      WidgetList _widgets;
      WidgetList::iterator _focus;
      void _set_focus(WidgetList::iterator new_focus);

      template <typename P> struct DerefLess {
        bool operator()(const P& x, const P& y) {
          return *x < *y;
        }
      };

      typedef std::map<WidgetList::iterator, Box, DerefLess<WidgetList::iterator> > WidgetRegionMap;
      WidgetRegionMap& _get_regions();
      WidgetRegionMap _widget_regions;
      bool _widget_regions_dirty;

    public:
      Menu(int width, int widget_height, int padding, Vector center_offset = Vector(0,0,0)) :
        _width(width), _widget_height(widget_height), _padding(padding),
        _center_offset(center_offset), _widgets(), _focus(_widgets.end()),
        _widget_regions_dirty(true)
      {}

      void add_widget(const boost::shared_ptr<Widget>& widget);
      void unfocus() { _set_focus(_widgets.end()); }

      void process();
      void draw();
  };
}

#endif
