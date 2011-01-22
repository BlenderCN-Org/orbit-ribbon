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

#include <boost/foreach.hpp>
#include <string>
#include <list>

#include "background.h"
#include "control_settings_menu_mode.h"
#include "display.h"
#include "font.h"
#include "globals.h"

const boost::array<ControlSettingsMenuMode::AxisBinding, 6>
ControlSettingsMenuMode::AXIS_BOUND_ACTION_NAMES = { {
  AxisBinding(ORSave::AxisBoundAction::TranslateX, "Move Left & Right"),
  AxisBinding(ORSave::AxisBoundAction::TranslateY, "Move Up & Down"),
  AxisBinding(ORSave::AxisBoundAction::TranslateZ, "Move Forward & Back"),
  AxisBinding(ORSave::AxisBoundAction::RotateY, "Turn Left & Right"),
  AxisBinding(ORSave::AxisBoundAction::RotateX, "Turn Up & Down"),
  AxisBinding(ORSave::AxisBoundAction::RotateZ, "Roll Left & Right")
} };

const boost::array<ControlSettingsMenuMode::ButtonBinding, 5>
ControlSettingsMenuMode::BUTTON_BOUND_ACTION_NAMES = { {
  ButtonBinding(ORSave::ButtonBoundAction::Confirm, "Menu: Confirm", false),
  ButtonBinding(ORSave::ButtonBoundAction::Cancel, "Menu: Cancel", false),
  ButtonBinding(ORSave::ButtonBoundAction::Pause, "Pause", false),
  ButtonBinding(ORSave::ButtonBoundAction::ForceQuit, "Force Quit", false, false),
  ButtonBinding(ORSave::ButtonBoundAction::ResetNeutral, "Reset Input Neutrals", false, false),
} };

ControlSettingsMenuMode::BindingRowWidgets ControlSettingsMenuMode::add_row_common(const Binding& binding) {
  BindingRowWidgets w;
  w.keyboard_binding.reset(binding.can_set_kbd_mouse ? (GUI::Widget*)(new GUI::Button()) : (GUI::Widget*)(new GUI::Label));
  w.mouse_binding.reset(binding.can_set_kbd_mouse ? (GUI::Widget*)(new GUI::Button()) : (GUI::Widget*)(new GUI::Label));
  w.gamepad_binding.reset(binding.can_set_gamepad ? (GUI::Widget*)(new GUI::Button()) : (GUI::Widget*)(new GUI::Label));

  _grid.add_row();
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label(binding.name, 0.0)));
  _grid.add_cell(w.keyboard_binding);
  _grid.add_cell(w.mouse_binding);
  _grid.add_cell(w.gamepad_binding);

  return w;
}

void ControlSettingsMenuMode::add_axis_binding_row(const AxisBinding& binding, const Channel& chan) {
  BindingRowWidgets w = add_row_common(binding);
}

void ControlSettingsMenuMode::add_button_binding_row(const ButtonBinding& binding, const Channel& chan) {
  BindingRowWidgets w = add_row_common(binding);
}

ControlSettingsMenuMode::ControlSettingsMenuMode(bool at_main_menu) :
  _grid(700, 20, 12), _at_main_menu(at_main_menu)
{
  _grid.add_row();
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::BlankWidget()));
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("Keyboard")));
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("Mouse")));
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("Gamepad/Joystick")));

  BOOST_FOREACH(const AxisBinding& binding, AXIS_BOUND_ACTION_NAMES) {
    const Channel& channel = Input::get_axis_ch(binding.action);
    add_axis_binding_row(binding, channel);
  }

  BOOST_FOREACH(const ButtonBinding& binding, BUTTON_BOUND_ACTION_NAMES) {
    const Channel& channel = Input::get_button_ch(binding.action);
    add_button_binding_row(binding, channel);
  }

  _grid.add_row(); // Blank separator row

  _grid.add_row(true);
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Button("Done")));
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Button("Reset All to Defaults")));
}

bool ControlSettingsMenuMode::handle_input() {
  _grid.process();

  if (Input::get_button_ch(ORSave::ButtonBoundAction::Cancel).matches_frame_events()) {
    Globals::mode_stack->next_frame_pop_mode();
  } else {
    BOOST_FOREACH(SDL_Event& e, Globals::frame_events) {
      if (e.type == SDL_USEREVENT) {
        if (e.user.code == GUI::WIDGET_CLICKED) {
          Globals::mode_stack->next_frame_push_mode(boost::shared_ptr<Mode>(new RebindingDialogMenuMode(
            1, "X", "Shot Web"
          )));
        }
      }
    }
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

bool RebindingDialogMenuMode::handle_input() {
  if (Input::get_button_ch(ORSave::ButtonBoundAction::Cancel).matches_frame_events()) {
    Globals::mode_stack->next_frame_pop_mode();
  }

  return true;
}

const Size REBINDING_DIALOG_SIZE(400,400);
const int REBINDING_DIALOG_FONT_HEIGHT = 20;
void RebindingDialogMenuMode::draw_2d(bool top __attribute__ ((unused))) {
  GUI::draw_box(Box(Point(0,0), Display::get_screen_size()), 0.3, 0.3, 0.3, 0.4);

  Box dialog_area(Point(0,0) + (Display::get_screen_size() - REBINDING_DIALOG_SIZE)/2, REBINDING_DIALOG_SIZE);
  GUI::draw_diamond_box(dialog_area, 0, 0, 0, 0.95);

  int text_width = Globals::sys_font->get_width(REBINDING_DIALOG_FONT_HEIGHT, _binding_desc);
  Globals::sys_font->draw(
    dialog_area.top_left + Vector((dialog_area.size.x - text_width)/2, REBINDING_DIALOG_FONT_HEIGHT/2),
    REBINDING_DIALOG_FONT_HEIGHT,
    _binding_desc
  );
}

bool AxisRebindingDialogMenuMode::handle_input() {
  return RebindingDialogMenuMode::handle_input();
}

bool ButtonRebindingDialogMenuMode::handle_input() {
  return RebindingDialogMenuMode::handle_input();
}
