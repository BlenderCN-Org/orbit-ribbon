/*
app.h: Header of the App class
App is responsible for the main frame loop and calling all the other parts of the program.

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

#ifndef ORBIT_RIBBON_APP_H
#define ORBIT_RIBBON_APP_H

#include <string>
#include <vector>

class App {
  public:
    static void run(const std::vector<std::string>& arguments);
    
    // To just load the base objects and sky for an area, specify 0 for mission
    static void load_mission(unsigned int area_num, unsigned int mission);
    
  private:
    static void init(const std::vector<std::string>& arguments, bool display_mode_reset);
    static void deinit();
    static void frame_loop();
};

#endif
