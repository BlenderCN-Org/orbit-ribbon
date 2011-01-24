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
#include "saving.h"

const boost::array<AxisActionDesc, 6>
ControlSettingsMenuMode::AXIS_BOUND_ACTION_NAMES = { {
  AxisActionDesc(ORSave::AxisBoundAction::TranslateX, "Move", "Left", "Right"),
  AxisActionDesc(ORSave::AxisBoundAction::TranslateY, "Move", "Up", "Down"),
  AxisActionDesc(ORSave::AxisBoundAction::TranslateZ, "Move", "Forward", "Back"),
  AxisActionDesc(ORSave::AxisBoundAction::RotateY, "Turn", "Left", "Right"),
  AxisActionDesc(ORSave::AxisBoundAction::RotateX, "Turn", "Up", "Down"),
  AxisActionDesc(ORSave::AxisBoundAction::RotateZ, "Roll", "Left", "Right")
} };

const boost::array<ButtonActionDesc, 5>
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
            GUI::Button* b = (GUI::Button*)w;
            Globals::mode_stack->next_frame_push_mode(boost::shared_ptr<Mode>(
              new RebindingDialogMenuMode(b->get_label(), &(bind_widget_iter->second))
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
  for (std::map<GUI::Widget*, BindingDesc>::iterator i = _binding_widgets.begin(); i != _binding_widgets.end(); ++i) {
    unsigned int desc_type = 0;
    switch (i->second.dev) {
      case ORSave::InputDeviceNameType::Keyboard:
        desc_type = CHANNEL_DESC_TYPE_KEYBOARD;
        break;
      case ORSave::InputDeviceNameType::Mouse:
        desc_type = CHANNEL_DESC_TYPE_MOUSE;
        break;
      case ORSave::InputDeviceNameType::Gamepad:
        desc_type = CHANNEL_DESC_TYPE_GAMEPAD;
        break;
      default:
        throw GameException("Unknown input device when populating control settings menu mode");
        break;
    }

    std::string cur_setting;
    const AxisActionDesc* adesc = dynamic_cast<const AxisActionDesc*>(i->second.action_desc);
    const ButtonActionDesc* bdesc = dynamic_cast<const ButtonActionDesc*>(i->second.action_desc);
    if (adesc) {
      cur_setting = Input::get_axis_ch(adesc->action).desc(desc_type);
    } else if (bdesc) {
      cur_setting = Input::get_button_ch(bdesc->action).desc(desc_type);
    } else {
      throw GameException("Failed to dyn cast binding in control settings mode");
    }

    GUI::Button* btn = dynamic_cast<GUI::Button*>(i->first);
    GUI::Label* lbl = dynamic_cast<GUI::Label*>(i->first);
    if (btn) {
      btn->set_label(cur_setting);
    } else if (lbl) {
      lbl->set_label(cur_setting);
    } else {
      throw GameException("Unknown widget in _binding_widgets of control settings mode");
    }
  }
}

RebindingDialogMenuMode::RebindingDialogMenuMode(const std::string& old_value, const BindingDesc* binding_desc) :
  _old_value(old_value), _binding_desc(binding_desc), _config_dev(NULL)
{
  _axis_mode = (dynamic_cast<const AxisActionDesc*>(binding_desc->action_desc) != NULL);

  std::string devname;
  switch (_binding_desc->dev) {
    case ORSave::InputDeviceNameType::Keyboard:
      devname = "keyboard";
      _instruction = "Press a key!";
      break;
    case ORSave::InputDeviceNameType::Mouse:
      devname = "mouse";
      if (_axis_mode) {
        _instruction = "Move the mouse, or press a mouse button!";
      } else {
        _instruction = "Press a mouse button!";
      }
      break;
    case ORSave::InputDeviceNameType::Gamepad:
      devname = "gamepad/joystick";
      if (_axis_mode) {
        _instruction = "Press a button or move a stick!";
      } else {
        _instruction = "Press a button!";
      }
      break;
    default:
      throw GameException("Unknown input device name type in rdmm ctor");
      break;
  }
  _title = "Assign " + devname + " mapping for:";

  try {
    _config_dev = &Saving::get_input_device(_binding_desc->dev);
  } catch (const NoSuchDeviceException& e) {
    // If it's just that no gamepad has been plugged in, no big deal
    // Note that whether or not there's an existing gamepad config, we still want to open the dialog
    // This allows the user to clear gamepad bindings even if it's not currently plugged in
    if (binding_desc->dev != ORSave::InputDeviceNameType::Gamepad) {
      throw;
    }
  }

  if (_config_dev) {
    _axis_bind_iter = _config_dev->axis_bind().end();
    _button_bind_iter = _config_dev->button_bind().end();

    if (_axis_mode) {
      ORSave::AxisBoundAction a = dynamic_cast<const AxisActionDesc*>(binding_desc->action_desc)->action;
      for (ORSave::InputDeviceType::axis_bind_iterator i = _config_dev->axis_bind().begin(); i != _config_dev->axis_bind().end(); ++i) {
        if (i->action() == a) {
          _axis_bind_iter = i;
          break;
        }
      }
    } else {
      ORSave::ButtonBoundAction a = dynamic_cast<const ButtonActionDesc*>(binding_desc->action_desc)->action;
      for (ORSave::InputDeviceType::button_bind_iterator i = _config_dev->button_bind().begin(); i != _config_dev->button_bind().end(); ++i) {
        if (i->action() == a) {
          _button_bind_iter = i;
          break;
        }
      }
    }
  }
}

bool RebindingDialogMenuMode::handle_input() {
  BOOST_FOREACH(SDL_Event& event, Globals::frame_events) {
    bool got_match = false;
    switch (event.type) {
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          Globals::mode_stack->next_frame_pop_mode();
          got_match = true;
        } else if (event.key.keysym.sym == SDLK_DELETE) {
          if (_config_dev) {
            if (_axis_mode && _axis_bind_iter != _config_dev->axis_bind().end()) {
              _config_dev->axis_bind().erase(_axis_bind_iter);
            } else if (!_axis_mode && _button_bind_iter != _config_dev->button_bind().end()) {
              _config_dev->button_bind().erase(_button_bind_iter);
            }
          }
          Input::set_channels_from_config();
          Globals::mode_stack->next_frame_pop_mode();
          got_match = true;
        } else if (_binding_desc->dev == ORSave::InputDeviceNameType::Keyboard) {
        }
        break;
      case SDL_MOUSEBUTTONDOWN:
        if (_binding_desc->dev == ORSave::InputDeviceNameType::Mouse) {
        }
        break;
      case SDL_JOYBUTTONDOWN:
        if (_binding_desc->dev == ORSave::InputDeviceNameType::Gamepad) {
        }
        break;
      case SDL_MOUSEMOTION:
        if (_binding_desc->dev == ORSave::InputDeviceNameType::Mouse) {
        }
        break;
      case SDL_JOYAXISMOTION:
        if (_binding_desc->dev == ORSave::InputDeviceNameType::Gamepad) {
        }
        break;
      default:
        // Do nothing
        break;
    }
    if (got_match) { break; }
  }

  return true;
}

const Size REBINDING_DIALOG_SIZE(400,240);
const int REBINDING_DIALOG_MINOR_FONT_HEIGHT = 16;
const int REBINDING_DIALOG_MAJOR_FONT_HEIGHT = 24;
void RebindingDialogMenuMode::draw_2d(bool top __attribute__ ((unused))) {
  GUI::draw_box(Box(Point(0,0), Display::get_screen_size()), 0.3, 0.3, 0.3, 0.4);

  Box dialog_area(Point(0,0) + (Display::get_screen_size() - REBINDING_DIALOG_SIZE)/2, REBINDING_DIALOG_SIZE);
  GUI::draw_diamond_box(dialog_area, 0, 0, 0, 0.95);

  Point pos = dialog_area.top_left;
  pos.y += REBINDING_DIALOG_MINOR_FONT_HEIGHT*0.5;

  int text_width = Globals::sys_font->get_width(REBINDING_DIALOG_MINOR_FONT_HEIGHT, _title);
  Globals::sys_font->draw(pos + Vector((dialog_area.size.x - text_width)/2, 0), REBINDING_DIALOG_MINOR_FONT_HEIGHT, _title);
  pos.y += REBINDING_DIALOG_MINOR_FONT_HEIGHT*1.2;

  const std::string& action_name = _binding_desc->action_desc->name;
  text_width = Globals::sys_font->get_width(REBINDING_DIALOG_MAJOR_FONT_HEIGHT, action_name);
  Globals::sys_font->draw(pos + Vector((dialog_area.size.x - text_width)/2, 0), REBINDING_DIALOG_MAJOR_FONT_HEIGHT, action_name);
  pos.y += REBINDING_DIALOG_MAJOR_FONT_HEIGHT*2.5;

  text_width = Globals::sys_font->get_width(REBINDING_DIALOG_MAJOR_FONT_HEIGHT, _instruction);
  Globals::sys_font->draw(pos + Vector((dialog_area.size.x - text_width)/2, 0), REBINDING_DIALOG_MAJOR_FONT_HEIGHT, _instruction);
  pos.y += REBINDING_DIALOG_MAJOR_FONT_HEIGHT*2.5;

  if (_old_value.size() > 0) {
    static const std::string to_replace("Replacing old mapping:");
    text_width = Globals::sys_font->get_width(REBINDING_DIALOG_MINOR_FONT_HEIGHT, to_replace);
    Globals::sys_font->draw(pos + Vector((dialog_area.size.x - text_width)/2, 0), REBINDING_DIALOG_MINOR_FONT_HEIGHT, to_replace);
    pos.y += REBINDING_DIALOG_MINOR_FONT_HEIGHT*1.2;

    text_width = Globals::sys_font->get_width(REBINDING_DIALOG_MINOR_FONT_HEIGHT, _old_value);
    Globals::sys_font->draw(pos + Vector((dialog_area.size.x - text_width)/2, 0), REBINDING_DIALOG_MINOR_FONT_HEIGHT, _old_value);
    pos.y += REBINDING_DIALOG_MINOR_FONT_HEIGHT*1.2;

    static const std::string del_instr("Press [Delete] to clear this mapping");
    text_width = Globals::sys_font->get_width(REBINDING_DIALOG_MINOR_FONT_HEIGHT, del_instr);
    Globals::sys_font->draw(pos + Vector((dialog_area.size.x - text_width)/2, 0), REBINDING_DIALOG_MINOR_FONT_HEIGHT, del_instr);
    pos.y += REBINDING_DIALOG_MINOR_FONT_HEIGHT*1.2;

    static const std::string cancel_instr("Press [Escape] to leave it as is");
    text_width = Globals::sys_font->get_width(REBINDING_DIALOG_MINOR_FONT_HEIGHT, cancel_instr);
    Globals::sys_font->draw(pos + Vector((dialog_area.size.x - text_width)/2, 0), REBINDING_DIALOG_MINOR_FONT_HEIGHT, cancel_instr);
  } else {
    static const std::string cancel_instr("Press " + Input::get_button_ch(ORSave::ButtonBoundAction::Cancel).desc() + " to leave it unmapped");
    text_width = Globals::sys_font->get_width(REBINDING_DIALOG_MINOR_FONT_HEIGHT, cancel_instr);
    Globals::sys_font->draw(pos + Vector((dialog_area.size.x - text_width)/2, 0), REBINDING_DIALOG_MINOR_FONT_HEIGHT, cancel_instr);
  }
}
