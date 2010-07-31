/*
main_menu_mode.cpp: Implementation for the MainMenuMode class.
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

#include <boost/lexical_cast.hpp>

#include "display.h"
#include "debug.h"
#include "main_menu_mode.h"
#include "gloo.h"
#include "gui.h"

MainMenuMode::MainMenuMode() :
  _cursor(GLOOTexture::load("cursor.png"))
{
  resumed();
}

void MainMenuMode::pre_clear() {
  glClearColor(0.0, 0.0, 0.0, 0.0);
}

void MainMenuMode::draw_2d() {
  if (_resumed) {
    SDL_WarpMouse(Display::get_screen_width()/2, Display::get_screen_height()/2);
    _resumed = false;
  }
  
  int x, y;
  SDL_GetMouseState(&x, &y);
  _cursor->draw_2d(Point(x - _cursor->get_width()/2, y - _cursor->get_width()/2));
}

void MainMenuMode::suspended() {
}

void MainMenuMode::resumed() {
  _resumed = true;
}
