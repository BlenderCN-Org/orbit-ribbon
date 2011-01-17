/*
control_settings_menu_mode.cpp: Implementation for the mode classes for configuring controls.

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

#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <list>

#include "autoxsd/save.h"
#include "background.h"
#include "control_settings_menu_mode.h"
#include "globals.h"
#include "input.h"

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

const boost::array<Binding<AAction>, 6> AXIS_BOUND_ACTION_NAMES = { {
  Binding<AAction>(ORSave::AxisBoundAction::TranslateX, "Move Left & Right"),
  Binding<AAction>(ORSave::AxisBoundAction::TranslateY, "Move Up & Down"),
  Binding<AAction>(ORSave::AxisBoundAction::TranslateZ, "Move Forward & Back"),
  Binding<AAction>(ORSave::AxisBoundAction::RotateY, "Turn Left & Right"),
  Binding<AAction>(ORSave::AxisBoundAction::RotateX, "Turn Up & Down"),
  Binding<AAction>(ORSave::AxisBoundAction::RotateZ, "Roll Left & Right")
} };

const boost::array<Binding<BAction>, 5> BUTTON_BOUND_ACTION_NAMES = { {
  Binding<BAction>(ORSave::ButtonBoundAction::Confirm, "Menu: Confirm", false),
  Binding<BAction>(ORSave::ButtonBoundAction::Cancel, "Menu: Cancel", false),
  Binding<BAction>(ORSave::ButtonBoundAction::Pause, "Pause", false),
  Binding<BAction>(ORSave::ButtonBoundAction::ForceQuit, "Force Quit", false, false),
  Binding<BAction>(ORSave::ButtonBoundAction::ResetNeutral, "Reset Input Neutrals", false, false),
} };

ControlSettingsMenuMode::ControlSettingsMenuMode(bool at_main_menu) :
  _grid(700, 22, 8), _at_main_menu(at_main_menu)
{
  _grid.add_row();
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::BlankWidget()));
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("Keyboard")));
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("Mouse")));
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("Gamepad/Joystick")));

  BOOST_FOREACH(const Binding<AAction>& action, AXIS_BOUND_ACTION_NAMES) {
    _grid.add_row();
    _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label(action.name, 0.0)));

    if (action.can_set_kbd_mouse) {
      _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Button("ABC123")));
      _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Button("ABC123")));
    } else {
      _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("ABC123")));
      _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("ABC123")));
    }

    if (action.can_set_gamepad) {
      _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Button("ABC123")));
    } else {
      _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("ABC123")));
    }
  }

  BOOST_FOREACH(const Binding<BAction>& action, BUTTON_BOUND_ACTION_NAMES) {
    _grid.add_row();
    _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label(action.name, 0.0)));

    if (action.can_set_kbd_mouse) {
      _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Button("ABC123")));
      _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Button("ABC123")));
    } else {
      _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("ABC123")));
      _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("ABC123")));
    }

    if (action.can_set_gamepad) {
      _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Button("ABC123")));
    } else {
      _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("ABC123")));
    }
  }

  _grid.add_row();

  _grid.add_row(true);
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Button("Done")));
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Button("Reset All to Defaults")));
}

bool ControlSettingsMenuMode::handle_input() {
  _grid.process();

  if (Input::get_button_ch(ORSave::ButtonBoundAction::Cancel).matches_frame_events()) {
    Globals::mode_stack->next_frame_pop_mode();
  }

  return true;
}

void ControlSettingsMenuMode::draw_3d_far(bool top __attribute__ ((unused))) {
  if (_at_main_menu) {
    Globals::bg->draw_starbox();
    Globals::bg->draw_objects();
  }
}

void ControlSettingsMenuMode::draw_2d(bool top __attribute__ ((unused))) {
  _grid.draw(true);
}

void ControlSettingsMenuMode::now_at_top() {
}
