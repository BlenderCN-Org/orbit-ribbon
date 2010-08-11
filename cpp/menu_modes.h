/*
main_modes.cpp: Header for the various menu mode classes.
These handle the menu screens used to start the game, choose a level, set options, etc.

Copyright 2009 David Simon. You can reach me at david.mike.simon@gmail.com

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

#ifndef ORBIT_RIBBON_MAIN_MENU_MODE_H
#define ORBIT_RIBBON_MAIN_MENU_MODE_H

#include <boost/shared_ptr.hpp>
#include <SDL/SDL.h>

#include "gloo.h"
#include "gui.h"
#include "mode.h"

class MenuMode : public Mode {
  private:
    GUI::SimpleMenu _simple_menu;
  
  protected:
    void add_entry(const std::string& name, const std::string& label);
  
  public:
    MenuMode(int menu_width, int btn_height, int padding);
    
    bool simulation_disabled() { return true; }
    bool mouse_cursor_enabled() { return true; }
    
    bool handle_input();
    void draw_2d(bool top);
    
    void pushed_below_top();
    
    virtual void handle_menu_selection(const std::string& item) =0;
};

class MainMenuMode : public MenuMode {
  public:
    MainMenuMode();
    void pre_clear(bool top);
    void handle_menu_selection(const std::string& item);
};

class AreaSelectMenuMode : public MenuMode {
  public:
    AreaSelectMenuMode();
    void pre_clear(bool top);
    void handle_menu_selection(const std::string& item);
};

class MissionSelectMenuMode : public MenuMode {
  private:
    unsigned int _area_num;
  
  public:
    MissionSelectMenuMode(unsigned int area_num);
    void pre_clear(bool top);
    void handle_menu_selection(const std::string& item);
};

class PauseMenuMode : public MenuMode {
  public:
    bool execute_after_lower_mode() { return true; }
    
    PauseMenuMode();
    void handle_menu_selection(const std::string& item);
};

#endif
