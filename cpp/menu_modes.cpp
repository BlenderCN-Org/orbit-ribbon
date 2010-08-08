/*
main_modes.cpp: Implementation for the various menu mode classes.
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

#include <boost/lexical_cast.hpp>

#include "menu_modes.h"

#include "app.h"
#include "display.h"
#include "gameplay_mode.h"
#include "globals.h"
#include "gloo.h"
#include "gui.h"

MainMenuMode::MainMenuMode() : _main_menu(180, 22, 8) {
  _main_menu.add_button("play", "Play");
  _main_menu.add_button("credits", "Credits");
  _main_menu.add_button("options", "Options");
  _main_menu.add_button("quit", "Quit");
}

bool MainMenuMode::handle_input() {
  _main_menu.process();
  std::string activated = _main_menu.get_activated_button();
  if (activated == "play") {
    App::load_mission(1, 1);
    Globals::mode_stack.next_frame_push_mode(boost::shared_ptr<Mode>(new GameplayMode()));
  } else if (activated == "quit") {
    Globals::mode_stack.next_frame_pop_mode();
  }
  
  return true;
}

void MainMenuMode::pre_clear(bool top __attribute__ ((unused))) {
  glClearColor(0.0, 0.0, 0.0, 0.0);
}

void MainMenuMode::draw_2d(bool top __attribute__ ((unused))) {
  _main_menu.draw();
}

void MainMenuMode::pushed_below_top() {
  _main_menu.reset_activation();
}
