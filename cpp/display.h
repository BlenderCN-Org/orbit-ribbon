/*
display.h: Header of the Display class
Display is responsible for interacting with SDL's video capabilities and for drawing each frame.

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

#ifndef ORBIT_RIBBON_DISPLAY_H
#define ORBIT_RIBBON_DISPLAY_H

#include <list>
#include <GL/glew.h>

#include "geometry.h"

class App;

struct VideoMode {
  Size size;
  bool fullscreen;

  VideoMode() : size(0,0), fullscreen(false) {}
  VideoMode(Size s, bool fs) : size(s), fullscreen(fs) {}

  bool operator==(const VideoMode& other);
  bool operator!=(const VideoMode& other) { return !(*this == other); }

  bool operator<(const VideoMode& other);
  bool operator<=(const VideoMode& other) { return *this < other || *this == other; }
  bool operator>(const VideoMode& other) { return !(*this <= other); }
  bool operator>=(const VideoMode& other) { return !(*this < other); }
};

struct SDL_VideoInfo;

class Display {
  private:
    static int screen_width, screen_height;
    static GLfloat screen_ratio;
    static const SDL_VideoInfo* vid_info;

  public:
    static int get_screen_width() { return screen_width; }
    static int get_screen_height() { return screen_height; }
    static Size get_screen_size() { return Size(screen_width, screen_height); }
    static float get_screen_ratio() { return screen_ratio; }
    static const SDL_VideoInfo* get_vid_info() { return vid_info; }

    static std::list<VideoMode> get_available_video_modes();

  private:
    static void init();

    static void screen_resize();
    
    friend class App;
};

#endif
