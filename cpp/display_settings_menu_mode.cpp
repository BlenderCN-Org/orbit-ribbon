/*
dispay_settings_menu_mode.cpp: Implementation for the menu mode class for setting video mode.
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
#include "globals.h"
#include "input.h"
#include "display_settings_menu_mode.h"
#include "saving.h"

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
