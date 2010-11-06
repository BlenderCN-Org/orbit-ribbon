/*
gui.h: Header for GUI related classes and functions.

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
  
  const float BUTTON_PASSIVE_COLOR[4] = { 0.5, 0.5, 0.8, 0.8 };
  const float BUTTON_FOCUSED_COLOR[4] = { 0.7, 0.7, 1.0, 1.0 };
  const float BUTTON_ACTIVATED_COLOR[4] = { 0.5, 0.8, 0.5, 1.0 };
  
  class FocusTracker {
    private:
      std::list<std::string> _region_order;
      std::map<std::string, Box> _focus_regions;
      std::map<std::string, Box>::const_iterator _focus_iter;
    
    public:
      FocusTracker() : _focus_regions(), _focus_iter(_focus_regions.end()) {}
      
      void add_region(const std::string& name, const Box& box);
      const Box* get_region(const std::string& name) const;
      void process();
      std::string get_current_focus() const;
  };
  
  class SimpleMenu {
    private:
      int _width, _btn_height, _padding;
      FocusTracker _focus_tracker;
      bool _filled_focus_tracker;
      Vector _center_offset;
      std::list<std::pair<std::string, std::string> > _entries;
      std::string _activated_entry;
    
    public:
      SimpleMenu(int width, int btn_height, int padding, Vector center_offset = Vector(0,0,0)) :
        _width(width), _btn_height(btn_height), _padding(padding), _filled_focus_tracker(false), _center_offset(center_offset) {}
      
      void add_button(const std::string& name, const std::string& label);
      
      void process();
      void draw();
      
      std::string get_activated_button() { return _activated_entry; }
      void reset_activation() { _activated_entry = ""; }
  };
}

#endif
