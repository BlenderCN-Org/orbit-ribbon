/*
dispay_settings_menu_mode.h: Header for the menu mode class for setting video mode.
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

#ifndef ORBIT_RIBBON_DISPLAY_SETTINGS_MENU_MODE_H
#define ORBIT_RIBBON_DISPLAY_SETTINGS_MENU_MODE_H

#include <boost/shared_ptr.hpp>
#include <map>

#include "display.h"
#include "gui.h"
#include "mode.h"

class DisplaySettingsMenuMode : public Mode {
  private:
    GUI::Menu _menu;
    boost::shared_ptr<GUI::Widget> _vsync_checkbox;
    boost::shared_ptr<GUI::Widget> _done_button;
    std::map<GUI::Button*, VideoMode> _mode_buttons_map;

  public:
    DisplaySettingsMenuMode();

    bool simulation_disabled() { return true; }
    bool mouse_cursor_enabled() { return true; }

    bool handle_input();
    void draw_3d_far(bool top);
    void draw_2d(bool top);
};

#endif
