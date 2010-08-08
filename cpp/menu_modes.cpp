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

void MenuMode::add_entry(const std::string& name, const std::string& label) {
  _simple_menu.add_button(name, label);
}

MenuMode::MenuMode(int menu_width, int btn_height, int padding) : _simple_menu(menu_width, btn_height, padding) {}

bool MenuMode::handle_input() {
  _simple_menu.process();
  std::string item = _simple_menu.get_activated_button();
  if (item != "") {
    handle_menu_selection(item);
  }
  return true;
}

void MenuMode::draw_2d(bool top __attribute__ ((unused))) {
  _simple_menu.draw();
}

void MenuMode::pushed_below_top() {
  _simple_menu.reset_activation();
}

MainMenuMode::MainMenuMode() : MenuMode(180, 22, 8) {
  add_entry("play", "Play");
  add_entry("credits", "Credits");
  add_entry("options", "Options");
  add_entry("quit", "Quit");
}

void MainMenuMode::pre_clear(bool top __attribute__ ((unused))) {
  glClearColor(0.0, 0.0, 0.0, 0.0);
}

void MainMenuMode::handle_menu_selection(const std::string& item) {
  if (item == "play") {
    App::load_mission(1, 1);
    Globals::mode_stack.next_frame_push_mode(boost::shared_ptr<Mode>(new GameplayMode()));
  } else if (item == "quit") {
    Globals::mode_stack.next_frame_pop_mode();
  }
}