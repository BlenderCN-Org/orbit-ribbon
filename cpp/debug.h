/*
debug.h: Header of the Debug class
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

#ifndef ORBIT_RIBBON_DEBUG_H
#define ORBIT_RIBBON_DEBUG_H

#include <string>

class Debug {
  private:
    static bool _logging;

    static void print(const std::string& msg);

  public:
    static void enable_logging();
    static void disable_logging();

    static void debug_msg(const std::string& msg);
    static void error_msg(const std::string& msg);
    static void status_msg(const std::string& msg);
};

#endif
