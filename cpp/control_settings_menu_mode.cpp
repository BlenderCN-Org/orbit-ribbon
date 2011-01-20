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

const boost::array<ControlSettingsMenuMode::Binding<ControlSettingsMenuMode::AAction>, 6>
ControlSettingsMenuMode::AXIS_BOUND_ACTION_NAMES = { {
  Binding<AAction>(ORSave::AxisBoundAction::TranslateX, "Move Left & Right"),
  Binding<AAction>(ORSave::AxisBoundAction::TranslateY, "Move Up & Down"),
  Binding<AAction>(ORSave::AxisBoundAction::TranslateZ, "Move Forward & Back"),
  Binding<AAction>(ORSave::AxisBoundAction::RotateY, "Turn Left & Right"),
  Binding<AAction>(ORSave::AxisBoundAction::RotateX, "Turn Up & Down"),
  Binding<AAction>(ORSave::AxisBoundAction::RotateZ, "Roll Left & Right")
} };

const boost::array<ControlSettingsMenuMode::Binding<ControlSettingsMenuMode::BAction>, 5>
ControlSettingsMenuMode::BUTTON_BOUND_ACTION_NAMES = { {
  Binding<BAction>(ORSave::ButtonBoundAction::Confirm, "Menu: Confirm", false),
  Binding<BAction>(ORSave::ButtonBoundAction::Cancel, "Menu: Cancel", false),
  Binding<BAction>(ORSave::ButtonBoundAction::Pause, "Pause", false),
  Binding<BAction>(ORSave::ButtonBoundAction::ForceQuit, "Force Quit", false, false),
  Binding<BAction>(ORSave::ButtonBoundAction::ResetNeutral, "Reset Input Neutrals", false, false),
} };

void ControlSettingsMenuMode::setup_grid() {
  _grid.clear();

  _grid.add_row();
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::BlankWidget()));
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("Keyboard")));
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("Mouse")));
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("Gamepad/Joystick")));

  BOOST_FOREACH(const Binding<AAction>& binding, AXIS_BOUND_ACTION_NAMES) {
    const Channel& channel = Input::get_axis_ch(binding.action);
    add_row(binding, channel);
  }

  BOOST_FOREACH(const Binding<BAction>& binding, BUTTON_BOUND_ACTION_NAMES) {
    const Channel& channel = Input::get_button_ch(binding.action);
    add_row(binding, channel);
  }

  _grid.add_row();

  _grid.add_row(true);
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Button("Done")));
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Button("Reset All to Defaults")));
}

ControlSettingsMenuMode::ControlSettingsMenuMode(bool at_main_menu) :
  _grid(700, 20, 12), _at_main_menu(at_main_menu)
{
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
  // FIXME: This clears the focus. Maybe I do need Button::set_value after all.
  setup_grid();
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
