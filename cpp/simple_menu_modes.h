/*
simpe_menu_modes.h: Header for the various simple menu mode classes.
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

#ifndef ORBIT_RIBBON_SIMPLE_MENU_MODES_H
#define ORBIT_RIBBON_SIMPLE_MENU_MODES_H

#include <boost/shared_ptr.hpp>
#include <SDL/SDL.h>
#include <map>
#include <string>


#include "gui.h"
#include "mode.h"

class SimpleMenuMode : public Mode {
  private:
    bool _draw_background;
    GUI::Menu _menu;
    std::map<GUI::Widget*, std::string> _entry_names;
  
  protected:
    void add_entry(const std::string& name, const std::string& label);
  
  public:
    SimpleMenuMode(bool draw_background, int menu_width, int btn_height, int padding, Vector center_offset = Vector(0,0,0));
    
    bool simulation_disabled() { return true; }
    bool mouse_cursor_enabled() { return true; }
    
    bool handle_input();
    void draw_3d_far(bool top);
    void draw_2d(bool top);
    
    virtual void handle_menu_selection(const std::string& item) =0;
};

class MainMenuMode : public SimpleMenuMode {
  public:
    MainMenuMode();
    void set_camera(bool top __attribute__ ((unused)));
    void handle_menu_selection(const std::string& item);
    void draw_2d(bool top);
};

class AreaSelectMenuMode : public SimpleMenuMode {
  public:
    AreaSelectMenuMode();
    void set_camera(bool top __attribute__ ((unused)));
    void handle_menu_selection(const std::string& item);
};

class MissionSelectMenuMode : public SimpleMenuMode {
  private:
    unsigned int _area_num;
  
  public:
    MissionSelectMenuMode(unsigned int area_num);
    void draw_3d_far(bool top);
    void set_camera(bool top);
    void handle_menu_selection(const std::string& item);
};

class PauseMenuMode : public SimpleMenuMode {
  public:
    bool execute_after_lower_mode() { return true; }
    
    PauseMenuMode();
    bool handle_input();
    void handle_menu_selection(const std::string& item);
};

class PostMissionMenuMode : public SimpleMenuMode {
  private:
    bool _won;

  public:
    bool execute_after_lower_mode() { return true; }

    PostMissionMenuMode(bool won);
    void handle_menu_selection(const std::string& item);
};

#endif
