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

#include "main_menu_mode.h"

#include "display.h"
#include "debug.h"
#include "globals.h"
#include "gloo.h"
#include "gui.h"
#include "mouse_cursor.h"

MainMenuMode::MainMenuMode() : _main_menu(180, 22, 8) {
  _main_menu.add_button("play", "Play");
  _main_menu.add_button("credits", "Credits");
  _main_menu.add_button("options", "Options");
  _main_menu.add_button("quit", "Quit");
}

void MainMenuMode::pre_clear(bool top __attribute__ ((unused))) {
  glClearColor(0.0, 0.0, 0.0, 0.0);
}

void MainMenuMode::draw_2d(bool top __attribute__ ((unused))) {
  _main_menu.process();
  _main_menu.draw();
}
