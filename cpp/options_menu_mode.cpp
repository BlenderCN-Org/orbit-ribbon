/*
options_menu_mode.cpp: Implementation for the menu mode class for setting options.
These handle the menu screens used to start the game, choose a level, etc.

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

#include "background.h"
#include "debug.h"
#include "globals.h"
#include "input.h"
#include "options_menu_mode.h"
#include "saving.h"

void OptionsMenuMode::_add_option(OptionWidgetId id, const boost::shared_ptr<GUI::Widget>& w) {
  _menu.add_widget(w);
  _widget_ids[&*w] = id;
  _id_widgets[id] = &*w;
}

void OptionsMenuMode::_init_widgets_from_config() {
  ORSave::ConfigType& c = Saving::get().config();

  ((GUI::Checkbox*)_id_widgets[OPTION_SHOW_FPS])->set_value(c.showFps());
  ((GUI::Checkbox*)_id_widgets[OPTION_DEBUG_PHYSICS])->set_value(c.debugPhysics());
  ((GUI::Checkbox*)_id_widgets[OPTION_INVERT_TRANSLATE_Y])->set_value(c.invertTranslateY());
  ((GUI::Checkbox*)_id_widgets[OPTION_INVERT_ROTATE_Y])->set_value(c.invertRotateY());

  ((GUI::Slider*)_id_widgets[OPTION_SFX_VOLUME])->set_value(c.soundEffectVolume());
  ((GUI::Slider*)_id_widgets[OPTION_MUSIC_VOLUME])->set_value(c.musicVolume());
  ((GUI::Slider*)_id_widgets[OPTION_MOUSE_SENSITIVITY])->set_value(c.mouseSensitivity());
}

OptionsMenuMode::OptionsMenuMode(bool at_main_menu) :
  _at_main_menu(at_main_menu),
  _menu(300, 22, 8)
{
  _add_option(OPTION_INVERT_ROTATE_Y, boost::shared_ptr<GUI::Widget>(new GUI::Checkbox("Invert vertical look")));
  _add_option(OPTION_INVERT_TRANSLATE_Y, boost::shared_ptr<GUI::Widget>(new GUI::Checkbox("Invert vertical movement")));
  _add_option(OPTION_CONTROLS, boost::shared_ptr<GUI::Widget>(new GUI::Button("Control bindings...")));
  _add_option(OPTION_MOUSE_SENSITIVITY, boost::shared_ptr<GUI::Widget>(new GUI::Slider("Mouse sensitivity in-game")));
  _menu.add_widget(boost::shared_ptr<GUI::Widget>(new GUI::BlankWidget()));
  _add_option(OPTION_SFX_VOLUME, boost::shared_ptr<GUI::Widget>(new GUI::Slider("Sound effect volume")));
  _add_option(OPTION_MUSIC_VOLUME, boost::shared_ptr<GUI::Widget>(new GUI::Slider("Music volume")));
  _menu.add_widget(boost::shared_ptr<GUI::Widget>(new GUI::BlankWidget()));
  if (_at_main_menu) {
    _add_option(OPTION_DISPLAY_MODE, boost::shared_ptr<GUI::Widget>(new GUI::Button("Display mode...")));
  }
  _add_option(OPTION_SHOW_FPS, boost::shared_ptr<GUI::Widget>(new GUI::Checkbox("Show FPS indicator")));
  _add_option(OPTION_DEBUG_PHYSICS, boost::shared_ptr<GUI::Widget>(new GUI::Checkbox("Show physics debugging info")));
  _menu.add_widget(boost::shared_ptr<GUI::Widget>(new GUI::BlankWidget()));
  _add_option(OPTION_SAVE, boost::shared_ptr<GUI::Widget>(new GUI::Button("Done")));
}


bool OptionsMenuMode::handle_input() {
  _menu.process();

  if (Input::get_button_ch(ORSave::ButtonBoundAction::Cancel).matches_frame_events()) {
    Saving::save();
    Globals::mode_stack.next_frame_pop_mode();
  } else {
    BOOST_FOREACH(SDL_Event& e, Globals::frame_events) {
      if (e.type == SDL_USEREVENT) {
        std::map<GUI::Widget*, OptionWidgetId>::iterator id = _widget_ids.find((GUI::Widget*)(e.user.data1));
        if (id != _widget_ids.end()) {
          if (e.user.code == GUI::WIDGET_CLICKED) {
            switch (id->second) {
              case OPTION_SAVE:
                Saving::save();
                Globals::mode_stack.next_frame_pop_mode();
                break;
              default:
                break;
            }
          } else if (e.user.code == GUI::WIDGET_VALUE_CHANGED) {
            ORSave::ConfigType& c = Saving::get().config();
            switch (id->second) {
              case OPTION_SHOW_FPS:
                c.showFps(static_cast<GUI::Checkbox*>(e.user.data1)->get_value());
                break;
              case OPTION_DEBUG_PHYSICS:
                c.debugPhysics(static_cast<GUI::Checkbox*>(e.user.data1)->get_value());
                break;
              case OPTION_SFX_VOLUME:
                c.soundEffectVolume(static_cast<GUI::Slider*>(e.user.data1)->get_value());
                break;
              case OPTION_MUSIC_VOLUME:
                c.musicVolume(static_cast<GUI::Slider*>(e.user.data1)->get_value());
                break;
              case OPTION_MOUSE_SENSITIVITY:
                c.mouseSensitivity(static_cast<GUI::Slider*>(e.user.data1)->get_value());
                break;
              case OPTION_INVERT_TRANSLATE_Y:
                c.invertTranslateY(static_cast<GUI::Checkbox*>(e.user.data1)->get_value());
                break;
              case OPTION_INVERT_ROTATE_Y:
                c.invertRotateY(static_cast<GUI::Checkbox*>(e.user.data1)->get_value());
                break;
              default:
                break;
            }
          }
        }
      }
    }
  }

  return true;
}

void OptionsMenuMode::draw_3d_far(bool top __attribute__ ((unused))) {
  if (_at_main_menu) {
    Globals::bg->draw_starbox();
    Globals::bg->draw_objects();
  }
}

void OptionsMenuMode::draw_2d(bool top __attribute__ ((unused))) {
  _menu.draw(true);
}

void OptionsMenuMode::pushed_below_top() {
  _menu.unfocus();
}

void OptionsMenuMode::now_at_top() {
  _init_widgets_from_config();
}

void DisplaySettingsMenuMode::_init_widgets_from_config() {
  ORSave::ConfigType& c = Saving::get().config();
}

DisplaySettingsMenuMode::DisplaySettingsMenuMode() : _menu(300, 22, 8) {
  BOOST_FOREACH(const VideoMode& mode, Display::get_available_video_modes()) {
  }
}

bool DisplaySettingsMenuMode::handle_input() {
}

void DisplaySettingsMenuMode::draw_3d_far(bool top) {
  Globals::bg->draw_starbox();
  Globals::bg->draw_objects();
}

void DisplaySettingsMenuMode::draw_2d(bool top) {
  _menu.draw(true);
}

void DisplaySettingsMenuMode::now_at_top() {
  _init_widgets_from_config();
}
