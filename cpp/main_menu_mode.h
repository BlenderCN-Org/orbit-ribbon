/*
main_menu_mode.h: Header for the MainMenuMode class.
The MainMenuMode class is the active Mode from when the program starts until a mission is begun.

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

class MainMenuMode : public Mode {
  private:
    GUI::SimpleMenu _main_menu;
    bool _resumed;
    
  public:
    MainMenuMode();
    
    void pre_clear();
    void draw_2d();
    
    void suspended();
    void resumed();
};

#endif
