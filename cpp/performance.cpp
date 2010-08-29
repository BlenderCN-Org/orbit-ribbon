/*
performance.cpp: Implementation for the Performance class, which tracks how smoothly the game is running

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

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <deque>

#include "constants.h"
#include "performance.h"

// How many ticks into the past performance are analyzed
const unsigned int PERF_TICKS_WINDOW = 1000;

struct FrameInfo {
  unsigned int total_ticks;
  unsigned int idle_ticks;
  FrameInfo(unsigned int t, unsigned int i) : total_ticks(t), idle_ticks(i) {}
};

std::deque<FrameInfo> frames;

void Performance::record_frame(unsigned int total_ticks, unsigned int idle_ticks) {
  frames.push_back(FrameInfo(total_ticks, idle_ticks));
  
  // Erase any FrameInfos that aren't needed to have a history PERF_TICKS_WINDOW ticks into the past
  unsigned int sum_t = 0;
  BOOST_FOREACH(const FrameInfo& f, frames) {
    sum_t += f.total_ticks;
  }
  while (1) {
    unsigned int front_ticks = frames.front().total_ticks;
    if ((sum_t - front_ticks) > PERF_TICKS_WINDOW) {
      sum_t -= front_ticks;
      frames.pop_front();
    } else {
      break;
    }
  }
}

std::string Performance::get_perf_info() {
  unsigned int sum_t = 0;
  unsigned int sum_i = 0;
  BOOST_FOREACH(const FrameInfo& f, frames) {
    sum_t += f.total_ticks;
    sum_i += f.idle_ticks;
  }
  
  if (sum_t < PERF_TICKS_WINDOW) {
    return std::string("CALCULATING FPS...   ");
  }
  
  return (boost::format("FPS:%4.2f IDLE:%4.2f%%")
    % (frames.size()*1000/float(sum_t))
    % (float(sum_i*100)/float(sum_t))
  ).str();
}
