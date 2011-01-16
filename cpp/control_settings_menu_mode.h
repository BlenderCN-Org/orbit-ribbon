/*
control_settings_menu_mode.h: Header for the mode classes for configuring controls.

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

#ifndef ORBIT_RIBBON_CONTROL_SETTINGS_MENU_MODE_H
#define ORBIT_RIBBON_CONTROL_SETTINGS_MENU_MODE_H

#include "gui.h"
#include "mode.h"

class ControlSettingsMenuMode : public Mode {
  private:
    bool _at_main_menu;
  
  public:
    ControlSettingsMenuMode(bool at_main_menu);
    
    bool execute_after_lower_mode() { return !_at_main_menu; }
    bool simulation_disabled() { return true; }
    bool mouse_cursor_enabled() { return true; }

    bool handle_input();
    void draw_3d_far(bool top);
    void draw_2d(bool top);
};

#endif
