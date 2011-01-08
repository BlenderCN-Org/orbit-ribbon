/*
debug.cpp: Implementation of the Debug class
Debug is responsible for logging and debugging output.

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

#include <boost/filesystem/fstream.hpp>
#include <string>
#include <iostream>

#include "constants.h"
#include "debug.h"
#include "globals.h"

bool Debug::_logging = false;

void Debug::print(const std::string& msg) {
#ifndef IN_WINDOWS
  std::cout << msg << std::endl << std::flush;
#endif

  if (_logging) {
    boost::filesystem::ofstream f(Globals::save_dir / LOG_FILENAME, std::ios_base::app | std::ios_base::out);
    f << msg << std::endl;
    f.close();
  }
}

void Debug::enable_logging() {
  _logging = true;
}

void Debug::debug_msg(const std::string& msg) {
  print("DEBUG: " + msg);
}

void Debug::error_msg(const std::string& msg) {
  print("ERROR: " + msg);
}

void Debug::status_msg(const std::string& msg) {
  print(msg);
}
