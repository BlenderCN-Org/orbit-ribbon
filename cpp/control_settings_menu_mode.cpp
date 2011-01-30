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
#include <cmath>
#include <string>
#include <list>

#include <boost/lexical_cast.hpp>
#include "debug.h"

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

const boost::array<ButtonActionDesc, 6>
ControlSettingsMenuMode::BUTTON_BOUND_ACTION_NAMES = { {
  ButtonActionDesc(ORSave::ButtonBoundAction::OrientForward, "Orient Forward"),
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

void ControlSettingsMenuMode::load_widget_labels() {
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

ControlSettingsMenuMode::ControlSettingsMenuMode(bool at_main_menu) :
  _grid(700, 20, 12), _at_main_menu(at_main_menu),
  _done_btn(new GUI::Button("Done")),
  _reset_btn(new GUI::Button("Reset All to Defaults"))
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
  _grid.add_cell(_done_btn);
  _grid.add_cell(_reset_btn);
}

bool ControlSettingsMenuMode::handle_input() {
  _grid.process();

  if (Input::get_button_ch(ORSave::ButtonBoundAction::Cancel).matches_frame_events()) {
    Globals::mode_stack->next_frame_pop_mode();
  } else {
    BOOST_FOREACH(SDL_Event& e, Globals::frame_events) {
      if (e.type == SDL_USEREVENT) {
        if (e.user.code == GUI::WIDGET_CLICKED) {
          if (e.user.data1 == (void*)(_done_btn.get())) {
            Globals::mode_stack->next_frame_pop_mode();
          } else if (e.user.data1 == (void*)(_reset_btn.get())) {
            Saving::get().config().inputDevice().clear();
            Input::load_config_presets();
            Input::set_channels_from_config();
            load_widget_labels();
          } else {
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
  load_widget_labels();
}

bool RebindingDialogMenuMode::is_clear_winning_axis(int x, int y, float min_delta) {
  if (std::max(std::fabs(x), std::fabs(x)) < min_delta) {
    // Reject small motions
    return false;
  }
  if (std::fabs(std::fabs(x) - std::fabs(y)) < min_delta) {
    // Reject ambiguously diagonal motions
    return false;
  }
  return true;
}

RebindingDialogMenuMode::RebindingDialogMenuMode(const std::string& old_value, const BindingDesc* binding_desc) :
  _old_value(old_value), _binding_desc(binding_desc), _config_dev(NULL), _calm(false),
  _detected_axis_num(-1), _detected_axis_num_2(-1),
  _detected_axis_negative(false), _detected_axis_negative_2(false)
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
    _config_dev = &*Saving::get_input_device(_binding_desc->dev);
  } catch (const NoSuchDeviceException& e) {
    // If it's just that no gamepad has been plugged in, we still want to open the dialog
    // This allows the user to clear gamepad bindings even if one's not currently plugged in
    if (binding_desc->dev != ORSave::InputDeviceNameType::Gamepad) {
      throw;
    }
  }

  if (_config_dev) {
    _axis_bind_iter = _config_dev->axis_bind().end();
    _button_bind_iter = _config_dev->button_bind().end();

    if (_axis_mode) {
      ORSave::AxisBoundAction a = static_cast<const AxisActionDesc*>(binding_desc->action_desc)->action;
      for (ORSave::InputDeviceType::axis_bind_iterator i = _config_dev->axis_bind().begin(); i != _config_dev->axis_bind().end(); ++i) {
        if (i->action() == a) {
          _axis_bind_iter = i;
          break;
        }
      }
    } else {
      ORSave::ButtonBoundAction a = static_cast<const ButtonActionDesc*>(binding_desc->action_desc)->action;
      for (ORSave::InputDeviceType::button_bind_iterator i = _config_dev->button_bind().begin(); i != _config_dev->button_bind().end(); ++i) {
        if (i->action() == a) {
          _button_bind_iter = i;
          break;
        }
      }
    }
  }
}

const float REBINDING_MINIMUM_MOUSE_REL_MOTION = 3.0;
const float REBINDING_MINIMUM_GAMEPAD_AXIS_VALUE = 0.3;
bool RebindingDialogMenuMode::handle_input() {
  // TODO: Ye flatulent hairless molerat gods but I need to refactor this whole method!

  // When considering joystick input for axis, we have to use Input, not the information in the SDL event.
  // Some gamepads have weird neutral positions (i.e. PS3 shoulder buttons rest at -1.0).
  // Our Input module treats value as difference-from-neutral, but SDL always assumes neutral at 1.0 for value.
  // Additionally, some gamepads have analog buttons that report as both button and axis.
  // In such cases, we need to always prefer the axis input over the button input.
  if (_axis_mode && _binding_desc->dev == ORSave::InputDeviceNameType::Gamepad) {
    bool calm_this_frame = true;

    boost::shared_ptr<Channel> null_chn = Input::get_null_channel();
    boost::shared_ptr<Channel> cand;
    int cand_gamepad_num = -1;
    int cand_axis_num = -1;

    const GamepadManager& gp_man = Input::get_gamepad_manager();
    for (Uint8 pad_num = 0; pad_num < gp_man.get_num_gamepads(); ++pad_num) {
      for (Uint8 axis_num = 0; axis_num < gp_man.get_num_axes(pad_num); ++axis_num) {
        boost::shared_ptr<Channel> axis = gp_man.axis_channel(pad_num, axis_num);
        float axis_value = std::fabs(axis->get_value());
        if (axis_value > REBINDING_MINIMUM_GAMEPAD_AXIS_VALUE) {
          calm_this_frame = false;
          if (!cand || dynamic_cast<GamepadButtonChannel*>(cand.get())) {
            cand = axis;
            cand_gamepad_num = pad_num;
            cand_axis_num = axis_num;
          } else if (dynamic_cast<GamepadAxisChannel*>(cand.get())) {
            float cand_value = std::fabs(cand->get_value());
            if (axis_value > cand_value) {
              cand = axis;
              cand_gamepad_num = pad_num;
              cand_axis_num = axis_num;
            }
          } else {
            throw GameException("Unknown type in gamepad binding candidate assign");
          }
        }
      }

      for (Uint8 button_num = 0; button_num < gp_man.get_num_buttons(pad_num); ++button_num) {
        boost::shared_ptr<Channel> button = gp_man.button_channel(pad_num, button_num);
        if (button->is_on()) {
          calm_this_frame = false;
          if (!cand) {
            cand = button;
            cand_gamepad_num = pad_num;
            cand_axis_num = button_num;
            break;
          }
        }
      }
    }

    if (_calm && cand) {
      std::auto_ptr<ORSave::BoundInputType> input;
      if (dynamic_cast<GamepadButtonChannel*>(cand.get())) {
        std::auto_ptr<ORSave::GamepadButtonInputType> button_input(new ORSave::GamepadButtonInputType);
        button_input->gamepadNum(cand_gamepad_num);
        button_input->buttonNum(cand_axis_num);
        input = button_input;
      } else if (dynamic_cast<GamepadAxisChannel*>(cand.get())) {
        std::auto_ptr<ORSave::GamepadAxisInputType> axis_input(new ORSave::GamepadAxisInputType);
        axis_input->gamepadNum(cand_gamepad_num);
        axis_input->axisNum(cand_axis_num);
        input = axis_input;
      } else {
        throw GameException("Unknown type in gamepad input assign from candidate");
      }

      bool negative = cand->get_value() < 0;
      if (_detected_input.get()) {
        // Don't map both sides of an axis action to the same input axis going in the same direction
        if (cand_axis_num != _detected_axis_num || negative != _detected_axis_negative) {
          calm_this_frame = false;
          _detected_input_2 = input;
          if (dynamic_cast<GamepadAxisChannel*>(cand.get())) {
            _detected_axis_num_2 = cand_axis_num;
            _detected_axis_negative_2 = negative;
          }
        }
      } else {
        calm_this_frame = false;
        _detected_input = input;
        if (dynamic_cast<GamepadAxisChannel*>(cand.get())) {
          _detected_axis_num = cand_axis_num;
          _detected_axis_negative = negative;
        }
      }
    }

    _calm = calm_this_frame;
  }

  BOOST_FOREACH(SDL_Event& event, Globals::frame_events) {
    switch (event.type) {
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          Globals::mode_stack->next_frame_pop_mode();
          return true;
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
          return true;
        } else if (_binding_desc->dev == ORSave::InputDeviceNameType::Keyboard) {
          if (event.key.keysym.sym == SDLK_PAUSE || event.key.keysym.sym == SDLK_F10 || event.key.keysym.sym == SDLK_F4) {
            // Ignore keys that correspond to some of the fixed default keyboard mappings
          } else {
            std::auto_ptr<ORSave::KeyInputType> input(new ORSave::KeyInputType);
            input->key(event.key.keysym.sym);

            if (_axis_mode && _detected_input.get()) {
              _detected_input_2 = input;
            } else {
              _detected_input = input;
            }
          }
        }
        break;
      case SDL_JOYBUTTONDOWN:
        if (!_axis_mode && _binding_desc->dev == ORSave::InputDeviceNameType::Gamepad) {
          // Why is it only this simple out of axis mode? Because some gamepads map a button and an axis to the same physical input.
          // In those cases, we want to make sure to prefer the axis over the button when assigning to _detected_input.
          std::auto_ptr<ORSave::GamepadButtonInputType> input(new ORSave::GamepadButtonInputType);
          input->gamepadNum(event.jbutton.which);
          input->buttonNum(event.jbutton.button);
          _detected_input = input;
        }
        break;
      case SDL_MOUSEBUTTONDOWN:
        if (_binding_desc->dev == ORSave::InputDeviceNameType::Mouse) {
          std::auto_ptr<ORSave::MouseButtonInputType> input(new ORSave::MouseButtonInputType);
          input->buttonNum(event.button.button);
          if (_axis_mode && _detected_input.get()) {
            _detected_input_2 = input;
          } else {
            _detected_input = input;
          }
        }
        break;
      case SDL_MOUSEMOTION:
        if (_axis_mode && _binding_desc->dev == ORSave::InputDeviceNameType::Mouse) {
          if (is_clear_winning_axis(event.motion.xrel, event.motion.yrel, REBINDING_MINIMUM_MOUSE_REL_MOTION)) {
            std::auto_ptr<ORSave::MouseMovementInputType> input(new ORSave::MouseMovementInputType);
            int axis_num = std::fabs(event.motion.xrel) > std::fabs(event.motion.yrel) ? 0 : 1;
            input->axisNum(axis_num);
            int value = axis_num == 0 ? event.motion.xrel : event.motion.yrel;
            bool negative = value < 0;
            // Don't map both sides of an axis action to the same input axis going in the same direction
            if (_detected_input.get() && (axis_num != _detected_axis_num || negative != _detected_axis_negative)) {
              _detected_input_2 = input;
              _detected_axis_num_2 = axis_num;
              _detected_axis_negative_2 = negative;
            } else {
              _detected_input = input;
              _detected_axis_num = axis_num;
              _detected_axis_negative = negative;
            }
          }
        }
        break;
      default:
        // Do nothing.
        break;
    }
  }

  if (_detected_input.get() && _detected_input_2.get() && typeid(_detected_input.get()) != typeid(_detected_input_2.get())) {
    // Don't create a pseudo-axis mapping with two different types of inputs
    _detected_input_2.reset();
  }

  if (_config_dev) {
    if (_axis_mode && _detected_input.get() && _detected_input_2.get()) {
      if (_axis_bind_iter != _config_dev->axis_bind().end()) {
        _config_dev->axis_bind().erase(_axis_bind_iter);
      }

      std::auto_ptr<ORSave::AxisBindType> binding(new ORSave::AxisBindType);
      binding->action(static_cast<const AxisActionDesc*>(_binding_desc->action_desc)->action);

      if (
        (Saving::get().config().invertTranslateY() && binding->action() == ORSave::AxisBoundAction::TranslateY)
        || (Saving::get().config().invertRotateY() && binding->action() == ORSave::AxisBoundAction::RotateX)
      ) {
        // Since the user was asked to do the input that causes the desired effect, and inversion is applied to this
        // action, we should swap the inputs they gave before saving the pre-inversion state to the config file.
        // Note that we apply inverted rotation to the RotateX action; user thinks of it as look up/down, but inside
        // the code we think of it as rotating about the x axis.
        std::auto_ptr<ORSave::BoundInputType> temp_input = _detected_input;
        _detected_input = _detected_input_2;
        _detected_input_2 = temp_input;

        int temp_axis_num = _detected_axis_num;
        _detected_axis_num = _detected_axis_num_2;
        _detected_axis_num_2 = temp_axis_num;

        bool temp_axis_negative = _detected_axis_negative;
        _detected_axis_negative = _detected_axis_negative_2;
        _detected_axis_negative_2 = temp_axis_negative;
      }

      if (_detected_axis_num >= 0 && _detected_axis_num == _detected_axis_num_2) {
        if (!dynamic_cast<ORSave::AxisBoundInputType*>(_detected_input.get())) {
          throw GameException("In RDMM, detected axis num is non-negative, but non-axis input type!");
        }
        std::auto_ptr<ORSave::AxisBoundInputType> axis_input(static_cast<ORSave::AxisBoundInputType*>(_detected_input.release()));

        if (_detected_axis_negative && !_detected_axis_negative_2) {
          binding->input(axis_input.release());
        } else if (!_detected_axis_negative && _detected_axis_negative_2) {
          std::auto_ptr<ORSave::InvertAxisInputType> invert_input(new ORSave::InvertAxisInputType);
          invert_input->axis(axis_input.release());
          binding->input(invert_input.release());
        } else {
          throw GameException("In RDMM, same axis supplied for two directions, but unexpected axis negation values");
        }
      } else {
        std::auto_ptr<ORSave::PseudoAxisInputType> pseudo_axis_input(new ORSave::PseudoAxisInputType);

        pseudo_axis_input->posInvert(_detected_axis_negative);
        pseudo_axis_input->negInvert(!_detected_axis_negative);
        pseudo_axis_input->negative(_detected_input.release());
        pseudo_axis_input->positive(_detected_input_2.release());

        binding->input(pseudo_axis_input.release());
      }

      _config_dev->axis_bind().push_back(binding.release());
      Input::set_channels_from_config();
      Globals::mode_stack->next_frame_pop_mode();
    } else if (!_axis_mode && _detected_input.get()) {
      if (_button_bind_iter != _config_dev->button_bind().end()) {
        _config_dev->button_bind().erase(_button_bind_iter);
      }

      std::auto_ptr<ORSave::ButtonBindType> binding(new ORSave::ButtonBindType);
      binding->action(static_cast<const ButtonActionDesc*>(_binding_desc->action_desc)->action);
      if (dynamic_cast<ORSave::ButtonBoundInputType*>(_detected_input.get())) {
        binding->input(static_cast<ORSave::ButtonBoundInputType*>(_detected_input.release()));
      } else {
        std::auto_ptr<ORSave::PseudoButtonInputType> pseudo_button_input(new ORSave::PseudoButtonInputType);
        pseudo_button_input->axis(static_cast<ORSave::AxisBoundInputType*>(_detected_input.release()));
        binding->input(pseudo_button_input.release());
      }

      _config_dev->button_bind().push_back(binding.release());
      Input::set_channels_from_config();
      Globals::mode_stack->next_frame_pop_mode();
    }
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

  std::string action_text;
  if (_axis_mode) {
    action_text = static_cast<const AxisActionDesc*>(_binding_desc->action_desc)->verb + " ";
    if (_detected_input.get()) {
      action_text += static_cast<const AxisActionDesc*>(_binding_desc->action_desc)->pos_name;
    } else {
      action_text += static_cast<const AxisActionDesc*>(_binding_desc->action_desc)->neg_name;
    }
  } else {
    action_text = _binding_desc->action_desc->name;
  }
  text_width = Globals::sys_font->get_width(REBINDING_DIALOG_MAJOR_FONT_HEIGHT, action_text);
  Globals::sys_font->draw(pos + Vector((dialog_area.size.x - text_width)/2, 0), REBINDING_DIALOG_MAJOR_FONT_HEIGHT, action_text);
  pos.y += REBINDING_DIALOG_MAJOR_FONT_HEIGHT*2.5;

  std::string instr_text;
  if (_axis_mode && _binding_desc->dev == ORSave::InputDeviceNameType::Gamepad && !_calm) {
    instr_text = "Please release all inputs.";
  } else {
    instr_text = _instruction;
  }
  text_width = Globals::sys_font->get_width(REBINDING_DIALOG_MAJOR_FONT_HEIGHT, instr_text);
  Globals::sys_font->draw(pos + Vector((dialog_area.size.x - text_width)/2, 0), REBINDING_DIALOG_MAJOR_FONT_HEIGHT, instr_text);
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
    static const std::string cancel_instr("Press [Escape] to leave it unmapped");
    text_width = Globals::sys_font->get_width(REBINDING_DIALOG_MINOR_FONT_HEIGHT, cancel_instr);
    Globals::sys_font->draw(pos + Vector((dialog_area.size.x - text_width)/2, 0), REBINDING_DIALOG_MINOR_FONT_HEIGHT, cancel_instr);
  }
}
