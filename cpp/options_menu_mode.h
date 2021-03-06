/*
options_menu_mode.h: Header for the menu mode class for setting options.
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

#ifndef ORBIT_RIBBON_OPTIONS_MENU_MODE_H
#define ORBIT_RIBBON_OPTIONS_MENU_MODE_H

#include <map>
#include <string>

#include "gui.h"
#include "mode.h"

class OptionsMenuMode : public Mode {
  private:
    enum OptionWidgetId {
      OPTION_SAVE,
      OPTION_SHOW_FPS,
      OPTION_DISPLAY_MODE,
      OPTION_DEBUG_PHYSICS,
      OPTION_SFX_VOLUME,
      OPTION_MUSIC_VOLUME,
      OPTION_MOUSE_SENSITIVITY,
      OPTION_INVERT_TRANSLATE_Y,
      OPTION_INVERT_ROTATE_Y,
      OPTION_CONTROLS
    };

    bool _at_main_menu;
    GUI::Menu _menu;
    std::map<GUI::Widget*, OptionWidgetId> _widget_ids;
    std::map<OptionWidgetId, GUI::Widget*> _id_widgets;
    void _add_option(OptionWidgetId id, const boost::shared_ptr<GUI::Widget>& w);

  public:
    OptionsMenuMode(bool at_main_menu);

    bool execute_after_lower_mode() { return !_at_main_menu; }
    bool simulation_disabled() { return true; }
    bool mouse_cursor_enabled() { return true; }

    bool handle_input();
    void draw_3d_far(bool top);
    void draw_2d(bool top);

    void now_at_top();
};

#endif
