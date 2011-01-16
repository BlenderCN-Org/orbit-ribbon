/*
display_settings_menu_mode.cpp: Implementation for the menu mode class for setting video mode.

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
#include <boost/format.hpp>
#include <string>

#include "background.h"
#include "display_settings_menu_mode.h"
#include "except.h"
#include "globals.h"
#include "input.h"
#include "saving.h"

const unsigned int MAXIMUM_MODES_LISTED = 14;

DisplaySettingsMenuMode::DisplaySettingsMenuMode() : _menu(300, 20, 5) {
  ORSave::ConfigType& c = Saving::get().config();

  BOOST_FOREACH(const VideoMode& mode, Display::get_available_video_modes()) {
    bool active = c.screenWidth() == mode.size.x && c.screenHeight() == mode.size.y && c.fullScreen() == mode.fullscreen;
    boost::format fmt("%1%%2%x%3% %4%%5%");
    fmt % (active ? "--> " : "");
    fmt % mode.size.x;
    fmt % mode.size.y;
    fmt % (mode.fullscreen ? "Fullscreen" : "Windowed");
    fmt % (active ? " <--" : "");
    boost::shared_ptr<GUI::Widget> btn(new GUI::Button(fmt.str()));
    _menu.add_widget(btn);
    _mode_buttons_map[static_cast<GUI::Button*>(&*btn)] = mode;

    if (_mode_buttons_map.size() >= MAXIMUM_MODES_LISTED) {
      break;
    }
  }

  _vsync_checkbox.reset(new GUI::Checkbox("V-Sync (prevents tearing, but can reduce framerate)", c.vSync()));
  _done_button.reset(new GUI::Button("Done"));

  _menu.add_widget(boost::shared_ptr<GUI::Widget>(new GUI::BlankWidget()));
  _menu.add_widget(_vsync_checkbox);
  _menu.add_widget(boost::shared_ptr<GUI::Widget>(new GUI::BlankWidget()));
  _menu.add_widget(_done_button);
}

bool DisplaySettingsMenuMode::handle_input() {
  _menu.process();

  if (Input::get_button_ch(ORSave::ButtonBoundAction::Cancel).matches_frame_events()) {
    Globals::mode_stack->next_frame_pop_mode();
  } else {
    bool setting_changed = false;
    ORSave::ConfigType& c = Saving::get().config();
    BOOST_FOREACH(SDL_Event& e, Globals::frame_events) {
      if (e.type == SDL_USEREVENT) {
        if (e.user.code == GUI::WIDGET_VALUE_CHANGED) {
          if ((GUI::Checkbox*)e.user.data1 == &*_vsync_checkbox) {
            c.vSync(static_cast<GUI::Checkbox*>(e.user.data1)->get_value());
            setting_changed = true;
          }
        } else if (e.user.code == GUI::WIDGET_CLICKED) {
          if ((GUI::Button*)e.user.data1 == &*_done_button) {
            Globals::mode_stack->next_frame_pop_mode();
          } else {
            std::map<GUI::Button*, VideoMode>::iterator mode_iter = _mode_buttons_map.find((GUI::Button*)(e.user.data1));
            if (mode_iter != _mode_buttons_map.end()) {
              VideoMode m = mode_iter->second;
              bool active = c.screenWidth() == m.size.x && c.screenHeight() == m.size.y && c.fullScreen() == m.fullscreen;
              if (!active) {
                // If we clicked on a mode other than the mode that's already active
                c.screenWidth(m.size.x);
                c.screenHeight(m.size.y);
                c.fullScreen(m.fullscreen);
                setting_changed = true;
              }
            }
          }
        }
      }
    }

    if (setting_changed) {
      Saving::save();
      throw DisplayModeResetException();
    }
  }

  return true;
}

void DisplaySettingsMenuMode::draw_3d_far(bool top __attribute__ ((unused))) {
  Globals::bg->draw_starbox();
  Globals::bg->draw_objects();
}

void DisplaySettingsMenuMode::draw_2d(bool top __attribute__ ((unused))) {
  _menu.draw(true);
}
