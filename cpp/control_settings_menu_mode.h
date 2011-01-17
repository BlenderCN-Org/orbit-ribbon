/*
control_settings_menu_mode.h: Header for the mode classes for configuring controls.

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

#ifndef ORBIT_RIBBON_CONTROL_SETTINGS_MENU_MODE_H
#define ORBIT_RIBBON_CONTROL_SETTINGS_MENU_MODE_H

#include <boost/array.hpp>

#include "autoxsd/save.h"
#include "gui.h"
#include "input.h"
#include "mode.h"

class ControlSettingsMenuMode : public Mode {
  private:
    GUI::Grid _grid;
    bool _at_main_menu;

    template <typename V> struct Binding {
      V action;
      std::string name;
      bool can_set_kbd_mouse;
      bool can_set_gamepad;

      Binding(const V& a, const std::string& n, bool km = true, bool g = true)
        : action(a), name(n), can_set_kbd_mouse(km), can_set_gamepad(g)
      {}
    };

    typedef ORSave::AxisBoundAction::value_type AAction;
    typedef ORSave::ButtonBoundAction::value_type BAction;

    static const boost::array<Binding<AAction>, 6> AXIS_BOUND_ACTION_NAMES;
    static const boost::array<Binding<BAction>, 5> BUTTON_BOUND_ACTION_NAMES;

    template <typename V> void add_row(const Binding<V>& binding, const Channel& chan) {
      _grid.add_row();
      _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label(binding.name, 0.0)));

      if (binding.can_set_kbd_mouse) {
        _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Button("")));
        _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Button(chan.desc())));
      } else {
        _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("")));
        _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label(chan.desc())));
      }

      if (binding.can_set_gamepad) {
        _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Button("")));
      } else {
        _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("")));
      }
    }

  public:
    ControlSettingsMenuMode(bool at_main_menu);
    
    bool execute_after_lower_mode() { return !_at_main_menu; }
    bool simulation_disabled() { return true; }
    bool mouse_cursor_enabled() { return true; }

    bool handle_input();
    void draw_3d_far(bool top);
    void draw_2d(bool top);

    void now_at_top();
};

#endif
