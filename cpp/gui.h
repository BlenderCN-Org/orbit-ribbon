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
  void draw_diamond_box(const Box& box, const float* color);

  enum UIEvent { WIDGET_GOT_FOCUS, WIDGET_LOST_FOCUS, WIDGET_CLICKED, WIDGET_VALUE_CHANGED };
  
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
      virtual bool focusable() { return true; };

      virtual void draw(const Box& box) =0;
      virtual void process(const Box& box) =0;
  };

  class BlankWidget : public Widget {
    public:
      void draw(const Box& box) {}
      void process(const Box& box) {}
      bool focusable() { return false; }
  };

  class Label : public Widget {
    private:
      std::string _label;
      float _x_align;

    public:
      Label(const std::string& label, float x_align = 0.5) : _label(label), _x_align(x_align) {}
      void draw(const Box& box);
      void process(const Box& box) {}
      bool focusable() { return false; }
  };

  class Button : public Widget {
    private:
      std::string _label;

    public:
      Button(const std::string& label) : _label(label) {}
      void draw(const Box& box);
      void process(const Box& box);
  };

  class Checkbox : public Widget {
    private:
      std::string _label;
      bool _value;

    public:
      Checkbox(const std::string& label, bool value = false) : _label(label), _value(value) {}
      void draw(const Box& box);
      void process(const Box& box);

      void set_value(bool value) { _value = value; }
      bool get_value() { return _value; }
  };

  class Slider : public Widget {
    private:
      std::string _label;
      float _value;

      Box _gauge_area(const Box& box);

    public:
      Slider(const std::string& label, float value = 0.0) :
        _label(label), _value(value)
      {}

      void draw(const Box& box);
      void process(const Box& box);

      void set_value(float value) { _value = value; }
      float get_value() { return _value; }
  };

  class WidgetLayout {
    protected:
      typedef std::map<Widget*, Box> RegionMap;

      void clear_region_map_cache();
      virtual void populate_widget_region_map(RegionMap& m) =0;

      const RegionMap& get_regions();

    private:
      RegionMap _cached_widget_regions;
      bool _widget_regions_dirty;
      Widget* _focus;

      bool _coverage_dirty;
      Box _coverage;

    public:
      WidgetLayout() : _widget_regions_dirty(true), _focus(NULL), _coverage_dirty(true) {}

      Widget* get_focus() { return _focus; }
      void set_focus(Widget* new_focus);
      void unfocus() { set_focus(NULL); }
      Box coverage();

      virtual void draw(bool frame);
      virtual void process();

      void clear() =0;
  };

  class Menu : public WidgetLayout {
    private:
      int _width, _widget_height, _padding;
      Vector _center_offset;

      typedef std::list<boost::shared_ptr<Widget> > WidgetList;
      WidgetList _widgets;

    protected:
      void populate_widget_region_map(WidgetLayout::RegionMap& m);

    public:
      Menu(int width, int widget_height, int padding, Vector center_offset = Vector(0,0,0)) :
        _width(width), _widget_height(widget_height), _padding(padding), _center_offset(center_offset)
      {}

      void add_widget(const boost::shared_ptr<Widget>& widget);
      void clear();

      void process();
  };

  class Grid : public WidgetLayout {
    private:
      struct Row {
        typedef std::list<boost::shared_ptr<Widget> > CellList;
        CellList cells;
        bool force_left_focus;

        Row(bool flf) : force_left_focus(flf) {}
      };

      int _width, _row_height, _padding;
      std::list<Row> _rows;

    protected:
      void populate_widget_region_map(WidgetLayout::RegionMap& m);

    public:
      Grid(int width, int row_height, int padding) :
        _width(width), _row_height(row_height), _padding(padding)
      {}

      void add_row(bool force_left_focus = false);
      void add_cell(const boost::shared_ptr<Widget>& widget);
      void clear();

      void process();
  };
}

#endif
