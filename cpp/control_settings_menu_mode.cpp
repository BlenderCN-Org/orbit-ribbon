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
#include "except.h"
#include "font.h"
#include "globals.h"
#include "mode.h"

const boost::array<ControlSettingsMenuMode::AxisActionDesc, 6>
ControlSettingsMenuMode::AXIS_BOUND_ACTION_NAMES = { {
  AxisActionDesc(ORSave::AxisBoundAction::TranslateX, "Move Left & Right"),
  AxisActionDesc(ORSave::AxisBoundAction::TranslateY, "Move Up & Down"),
  AxisActionDesc(ORSave::AxisBoundAction::TranslateZ, "Move Forward & Back"),
  AxisActionDesc(ORSave::AxisBoundAction::RotateY, "Turn Left & Right"),
  AxisActionDesc(ORSave::AxisBoundAction::RotateX, "Turn Up & Down"),
  AxisActionDesc(ORSave::AxisBoundAction::RotateZ, "Roll Left & Right")
} };

const boost::array<ControlSettingsMenuMode::ButtonActionDesc, 5>
ControlSettingsMenuMode::BUTTON_BOUND_ACTION_NAMES = { {
  ButtonActionDesc(ORSave::ButtonBoundAction::Confirm, "Menu: Confirm", false),
  ButtonActionDesc(ORSave::ButtonBoundAction::Cancel, "Menu: Cancel", false),
  ButtonActionDesc(ORSave::ButtonBoundAction::Pause, "Pause", false),
  ButtonActionDesc(ORSave::ButtonBoundAction::ForceQuit, "Force Quit", false, false),
  ButtonActionDesc(ORSave::ButtonBoundAction::ResetNeutral, "Reset Input Neutrals", false, false),
} };

void ControlSettingsMenuMode::add_row(const ActionDesc& action, const Channel& chan) {
  boost::shared_ptr<GUI::Widget> k_bind_widget(action.can_set_kbd_mouse ? (GUI::Widget*)(new GUI::Button()) : (GUI::Widget*)(new GUI::Label));
  boost::shared_ptr<GUI::Widget> m_bind_widget(action.can_set_kbd_mouse ? (GUI::Widget*)(new GUI::Button()) : (GUI::Widget*)(new GUI::Label));
  boost::shared_ptr<GUI::Widget> g_bind_widget(action.can_set_gamepad ? (GUI::Widget*)(new GUI::Button()) : (GUI::Widget*)(new GUI::Label));

  _grid.add_row();
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label(action.name, 0.0)));
  _grid.add_cell(k_bind_widget);
  _grid.add_cell(m_bind_widget);
  _grid.add_cell(g_bind_widget);

  _binding_widgets.insert(std::pair<GUI::Widget*, BindingDesc>(k_bind_widget.get(), BindingDesc(action, ORSave::InputDeviceNameType::Keyboard)));
  _binding_widgets.insert(std::pair<GUI::Widget*, BindingDesc>(m_bind_widget.get(), BindingDesc(action, ORSave::InputDeviceNameType::Mouse)));
  _binding_widgets.insert(std::pair<GUI::Widget*, BindingDesc>(g_bind_widget.get(), BindingDesc(action, ORSave::InputDeviceNameType::Gamepad)));
}


ControlSettingsMenuMode::ControlSettingsMenuMode(bool at_main_menu) :
  _grid(700, 20, 12), _at_main_menu(at_main_menu)
{
  _grid.add_row();
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::BlankWidget()));
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("Keyboard")));
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("Mouse")));
  _grid.add_cell(boost::shared_ptr<GUI::Widget>(new GUI::Label("Gamepad/Joystick")));

  BOOST_FOREACH(const AxisActionDesc& binding, AXIS_BOUND_ACTION_NAMES) {
    const Channel& channel = Input::get_axis_ch(binding.action);
    add_row(binding, channel);
  }

  BOOST_FOREACH(const ButtonActionDesc& binding, BUTTON_BOUND_ACTION_NAMES) {
    const Channel& channel = Input::get_button_ch(binding.action);
    add_row(binding, channel);
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
          GUI::Widget* w = (GUI::Widget*)(e.user.data1);
          std::map<GUI::Widget*, BindingDesc>::iterator bind_widget_iter = _binding_widgets.find(w);
          if (bind_widget_iter != _binding_widgets.end()) {
            Globals::mode_stack->next_frame_push_mode(boost::shared_ptr<Mode>(
              new RebindingDialogMenuMode("", &(bind_widget_iter->second))
            ));
          } else {
            throw GameException("Mysterious click event in control settings input handler!");
          }
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

RebindingDialogMenuMode::RebindingDialogMenuMode(const std::string& old_value, const BindingDesc* binding_desc) :
  _old_value(old_value), _binding_desc(binding_desc), _title("Rebinding: " + binding_desc->action_desc->name)
{
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

  int text_width = Globals::sys_font->get_width(REBINDING_DIALOG_FONT_HEIGHT, _title);
  Globals::sys_font->draw(
    dialog_area.top_left + Vector((dialog_area.size.x - text_width)/2, REBINDING_DIALOG_FONT_HEIGHT/2),
    REBINDING_DIALOG_FONT_HEIGHT,
    _title
  );
}
