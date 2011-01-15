/*
display.cpp: Implementation of the Display class
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

#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <GL/glew.h>
#include <SDL/SDL.h>

#include "autoinfo/version.h"
#include "background.h"
#include "constants.h"
#include "debug.h"
#include "display.h"
#include "except.h"
#include "globals.h"
#include "gui.h"
#include "saving.h"
#include "mode.h"
#include "mouse_cursor.h"
#include "performance.h"
#include "gloo.h"

SDL_Surface* screen;

int Display::screen_width = 0;
int Display::screen_height = 0;
GLfloat Display::screen_ratio = 0;
const SDL_VideoInfo* Display::vid_info = NULL;

const int SDL_VIDEO_FLAGS = SDL_OPENGL | SDL_GL_DOUBLEBUFFER;

bool VideoMode::operator==(const VideoMode& other) {
  return size == other.size && fullscreen == other.fullscreen;
}

bool VideoMode::operator<(const VideoMode& other) {
  if (fullscreen != other.fullscreen) {
    // Windowed modes come before fullscreen modes
    return !fullscreen;
  } else {
    // Sort by horizontal resolution than vertical resolution, higher first
    if (size.x != other.size.x) {
      return size.x > other.size.x;
    } else {
      return size.y > other.size.y;
    }
  }
}

const boost::array<Size, 7> STANDARD_RESOLUTIONS = { {
  Size(800,600),
  Size(1024,768),
  Size(1280,800),
  Size(1280,960),
  Size(1280,1024),
  Size(1400,1050),
  Size(1600,1200)
} };

std::list<VideoMode> Display::get_available_video_modes() {
  std::list<VideoMode> r;

  unsigned int largest_x = 0;
  unsigned int largest_y = 0;
  SDL_Rect** modes = SDL_ListModes(vid_info->vfmt, SDL_VIDEO_FLAGS | SDL_FULLSCREEN);
  if (modes == (SDL_Rect**)0) {
    throw GameException("Unable to retrieve video modes");
  } else if (modes == (SDL_Rect**)-1) {
    Debug::status_msg("FS video modes unrestricted? Weird. Defaulting to 800x600 windowed.");
    r.push_back(VideoMode(Size(800,600), false));
    return r;
  }

  // Fullscreen modes
  for (int i = 0; modes[i]; ++i) {
    if (modes[i]->w >= 800 && modes[i]->h >= 600) {
      r.push_back(VideoMode(Size(modes[i]->w, modes[i]->h), true));
      if (modes[i]->w > largest_x) { largest_x = modes[i]->w; }
      if (modes[i]->h > largest_y) { largest_y = modes[i]->h; }
    }
  }

  // Windowed modes
  BOOST_FOREACH(const Size& size, STANDARD_RESOLUTIONS) {
    if (size.x < largest_x && size.y < largest_y) {
      r.push_back(VideoMode(size, false));
    }
  }

  if (r.size() == 0) {
    throw GameException("No valid video modes found; perhaps your display is worse than 800x600");
  }

  r.sort();
  return r;
}

void Display::init() {
  bool full_screen = Saving::get().config().fullScreen();
  
  vid_info = SDL_GetVideoInfo();
  if (!vid_info) {
    throw GameException("Unable to query video info: " + std::string(SDL_GetError()));
  }
  
  if (Saving::get().config().screenWidth_present() && Saving::get().config().screenHeight_present()) {
    screen_width = Saving::get().config().screenWidth();
    screen_height = Saving::get().config().screenHeight();
  }
  
  if (screen_width == 0 || screen_height == 0 || !SDL_VideoModeOK(screen_width, screen_height, vid_info->vfmt->BitsPerPixel, SDL_VIDEO_FLAGS)) {
    Debug::status_msg("No acceptable resolution specified in save file, choosing new resolution...");
    
    // Need to come up with default values
    if (full_screen) {
      SDL_Rect** modes = SDL_ListModes(vid_info->vfmt, SDL_VIDEO_FLAGS | SDL_FULLSCREEN );
      if (modes == (SDL_Rect**)0) {
        Debug::status_msg("No fullscreen video modes available, disabling fullscreen mode for this run");
        full_screen = false;
        screen_width = 800;
        screen_height = 600;
      } else if (modes == (SDL_Rect**)-1) {
        Debug::status_msg("Fullscreen video modes are unrestricted (?)");
        screen_width = 800;
        screen_height = 600;
      } else {
        screen_width = modes[0]->w;
        screen_height = modes[0]->h;
      }
    } else {
      // If we're not fullscreen, just go with 800x600
      screen_width = 800;
      screen_height = 600;
    }
    
    Saving::get().config().screenWidth(screen_width);
    Saving::get().config().screenHeight(screen_height);
    Saving::get().config().fullScreen(full_screen);
    Saving::save();
  }
  
  screen_ratio = screen_width/screen_height;
  
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  if (Saving::get().config().vSync()) {
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
  }
  std::string win_title = std::string("Orbit Ribbon (") + APP_VERSION + std::string(")");
  SDL_WM_SetCaption(win_title.c_str(), win_title.c_str());
  //FIXME : Set window icon here
  
  Debug::status_msg(
    "Setting display mode to " +
    boost::lexical_cast<std::string>(screen_width) +
    "x" +
    boost::lexical_cast<std::string>(screen_height) +
    (full_screen ? " fullscreen" : " windowed")
  );
  
  screen = SDL_SetVideoMode(get_screen_width(), get_screen_height(), vid_info->vfmt->BitsPerPixel, SDL_VIDEO_FLAGS | (full_screen ? SDL_FULLSCREEN : 0));
  if (!screen) {
    throw GameException(std::string("Video mode set failed: ") + std::string(SDL_GetError()));
  }
  
  SDL_ShowCursor(SDL_DISABLE);
  SDL_WM_GrabInput(SDL_GRAB_ON); // "Captain! Scanners are detecting a Grabon warship in the area!" "Fire photon torpedoes!"
  
  glewExperimental = GL_TRUE; // My video card doesn't report ARB_map_buffer_range even though it's available, unless this is enabled
  GLenum glew_err = glewInit();
  if (glew_err != GLEW_OK) {
    throw GameException(std::string("GLEW init failed: " ) + std::string((const char*)glewGetErrorString((glew_err))));
  }
  
  if (!GLEW_VERSION_2_1) {
    throw GameException(std::string("Need OpenGL version 2.1 or greater"));
  }
  
  if (!GLEW_ARB_map_buffer_range) {
    throw GameException(std::string("Need OpenGL extension ARB_map_buffer_range"));
  }

  // Quadratic value arrived at emperically, works OK but I don't think it's very realistic
  float sprite_quadratic[] = { 0.0, 0.0, 0.0000015 };
  glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, sprite_quadratic);
  GLfloat sprite_sizes[2];
  glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, sprite_sizes);
  glPointParameterf(GL_POINT_SIZE_MIN, sprite_sizes[0]);
  glPointParameterf(GL_POINT_SIZE_MAX, sprite_sizes[1]);
  glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
  
  glEnable(GL_TEXTURE_2D);
  
  glShadeModel(GL_SMOOTH);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_CULL_FACE);
  
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_INDEX_ARRAY);
  
  screen_resize();
}

void Display::screen_resize() {
  screen_width = Display::get_screen_width();
  screen_height = Display::get_screen_height();
  screen_ratio = GLfloat(screen_width)/GLfloat(screen_height);
  
  glViewport(0, 0, screen_width, screen_height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
}
