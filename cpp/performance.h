/*
performance.h: Header for the Performance class, which tracks how smoothly the game is running

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

#ifndef ORBIT_RIBBON_PERFORMANCE_H
#define ORBIT_RIBBON_PERFORMANCE_H

#include <string>

class App;

class Performance {
  private:
    static void record_frame(unsigned int total_ticks, unsigned int idle_ticks);
    
    friend class App;
  
  public:
    static std::string get_perf_info();
};

#endif
