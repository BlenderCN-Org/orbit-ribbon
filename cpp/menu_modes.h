/*
main_modes.cpp: Header for the various menu mode classes.
These handle the menu screens used to start the game, choose a level, set options, etc.

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

#ifndef ORBIT_RIBBON_MAIN_MENU_MODE_H
#define ORBIT_RIBBON_MAIN_MENU_MODE_H

#include <boost/shared_ptr.hpp>
#include <SDL/SDL.h>

#include "gloo.h"
#include "gui.h"
#include "mode.h"

class MenuMode : public Mode {
  private:
    bool _draw_background;
    GUI::SimpleMenu _simple_menu;
  
  protected:
    void add_entry(const std::string& name, const std::string& label);
  
  public:
    MenuMode(bool draw_background, int menu_width, int btn_height, int padding, Vector center_offset = Vector(0,0,0));
    
    bool simulation_disabled() { return true; }
    bool mouse_cursor_enabled() { return true; }
    
    bool handle_input();
    void draw_3d_far(bool top);
    void draw_2d(bool top);
    
    void pushed_below_top();
    
    virtual void handle_menu_selection(const std::string& item) =0;
};

class MainMenuMode : public MenuMode {
 private:
    boost::shared_ptr<GLOOTexture> _title_tex;
    GLOOCamera _camera;

  public:
    MainMenuMode();
    const GLOOCamera* get_camera(bool top __attribute__ ((unused))) { return &_camera; }
    void handle_menu_selection(const std::string& item);
    void draw_2d(bool top);
};

class AreaSelectMenuMode : public MenuMode {
  private:
    GLOOCamera _camera;

  public:
    AreaSelectMenuMode();
    const GLOOCamera* get_camera(bool top __attribute__ ((unused))) { return &_camera; }
    void handle_menu_selection(const std::string& item);
};

class MissionSelectMenuMode : public MenuMode {
  private:
    unsigned int _area_num;
    GLOOCamera _camera;
  
  public:
    MissionSelectMenuMode(unsigned int area_num);
    void draw_3d_far(bool top);
    const GLOOCamera* get_camera(bool top);
    void handle_menu_selection(const std::string& item);
};

class PauseMenuMode : public MenuMode {
  public:
    bool execute_after_lower_mode() { return true; }
    
    PauseMenuMode();
    bool handle_input();
    void handle_menu_selection(const std::string& item);
};

class PostMissionMenuMode : public MenuMode {
  private:
    bool _won;

  public:
    bool execute_after_lower_mode() { return true; }

    PostMissionMenuMode(bool won);
    void handle_menu_selection(const std::string& item);
};

#endif
