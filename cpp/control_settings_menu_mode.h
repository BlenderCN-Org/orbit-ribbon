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

    struct Binding {
      std::string name;
      bool can_set_kbd_mouse;
      bool can_set_gamepad;
      Binding(const std::string& n, bool km, bool g) : name(n), can_set_kbd_mouse(km), can_set_gamepad(g) {}
    };

    typedef ORSave::AxisBoundAction::value_type AAction;
    typedef ORSave::ButtonBoundAction::value_type BAction;

    struct AxisBinding : public Binding {
      AAction action;
      AxisBinding(AAction a, const std::string& n, bool km = true, bool g = true) : Binding(n, km, g), action(a) {}
    };

    struct ButtonBinding : public Binding {
      BAction action;
      ButtonBinding(BAction a, const std::string& n, bool km = true, bool g = true) : Binding(n, km, g), action(a) {}
    };

    struct BindingRowWidgets {
      boost::shared_ptr<GUI::Widget> keyboard_binding;
      boost::shared_ptr<GUI::Widget> mouse_binding;
      boost::shared_ptr<GUI::Widget> gamepad_binding;
    };

    static const boost::array<AxisBinding, 6> AXIS_BOUND_ACTION_NAMES;
    static const boost::array<ButtonBinding, 5> BUTTON_BOUND_ACTION_NAMES;

    BindingRowWidgets add_row_common(const Binding& binding);
    void add_axis_binding_row(const AxisBinding& binding, const Channel& chan);
    void add_button_binding_row(const ButtonBinding& binding, const Channel& chan);

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

class RebindingDialogMenuMode : public Mode {
  protected:
    unsigned int _desc_type;
    std::string _old_value;
    std::string _binding_desc;

  public:
    RebindingDialogMenuMode(unsigned int desc_type, const std::string& old_value, const std::string& binding_desc) :
      _desc_type(desc_type), _old_value(old_value), _binding_desc("Rebinding Action: " +  binding_desc)
    {}

    bool execute_after_lower_mode() { return true; }
    bool simulation_disabled() { return true; }
    bool mouse_cursor_enabled() { return false; }

    bool handle_input();
    void draw_2d(bool top);
};

class AxisRebindingDialogMenuMode : public RebindingDialogMenuMode {
  private:
    ORSave::AxisBoundAction::value_type _action;

  public:
    AxisRebindingDialogMenuMode(
      ORSave::AxisBoundAction::value_type action,
      unsigned int desc_type, const std::string& old_value, const std::string& binding_desc
      ) : RebindingDialogMenuMode(desc_type, old_value, binding_desc), _action(action) 
    {}

    bool handle_input();
};

class ButtonRebindingDialogMenuMode : public RebindingDialogMenuMode {
  private:
    ORSave::ButtonBoundAction::value_type _action;
    std::string _neg_name, _pos_name;

  public:
    ButtonRebindingDialogMenuMode(
      ORSave::ButtonBoundAction::value_type action,
      const std::string& neg_name, const std::string& pos_name,
      unsigned int desc_type, const std::string& old_value, const std::string& binding_desc
      ) : RebindingDialogMenuMode(desc_type, old_value, binding_desc), _action(action), _neg_name(neg_name), _pos_name(pos_name)
    {}

    bool handle_input();
};

#endif
