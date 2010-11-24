/*
display.cpp: Implementation of the Display class
Display is responsible for interacting with SDL's video capabilities and for drawing each frame.

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
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
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

void Display::init() {
  bool full_screen = Saving::get().config().fullScreen().get();
  
  const SDL_VideoInfo* vid_info = SDL_GetVideoInfo();
  if (!vid_info) {
    throw GameException("Unable to query video info: " + std::string(SDL_GetError()));
  }
  
  GLint videoFlags;
  videoFlags  = SDL_OPENGL;
  videoFlags |= SDL_GL_DOUBLEBUFFER;
  
  if (Saving::get().config().screenWidth().present() && Saving::get().config().screenHeight().present()) {
    screen_width = Saving::get().config().screenWidth().get();
    screen_height = Saving::get().config().screenHeight().get();
  }
  
  if (screen_width == 0 || screen_height == 0 || !SDL_VideoModeOK(screen_width, screen_height, vid_info->vfmt->BitsPerPixel, videoFlags)) {
    Debug::status_msg("No acceptable resolution specified in save file, choosing new resolution...");
    
    // Need to come up with default values
    if (full_screen) {
      SDL_Rect** modes;
      modes = SDL_ListModes(vid_info->vfmt, videoFlags | SDL_FULLSCREEN );
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
    
    Saving::get().config().screenWidth().set(screen_width);
    Saving::get().config().screenHeight().set(screen_height);
    Saving::get().config().fullScreen().set(full_screen);
    Saving::save();
  }
  
  screen_ratio = screen_width/screen_height;
  
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  if (Saving::get().config().vSync().get()) {
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
  
  screen = SDL_SetVideoMode(get_screen_width(), get_screen_height(), vid_info->vfmt->BitsPerPixel, videoFlags | (full_screen ? SDL_FULLSCREEN : 0));
  if (!screen) {
    throw GameException(std::string("Video mode set failed: ") + std::string(SDL_GetError()));
  }
  
  SDL_ShowCursor(SDL_DISABLE);
  SDL_WM_GrabInput(SDL_GRAB_ON); // "Captain! Scanners are detecting a Grabon warship in the area!" "Fire photon torpedoes!"
  
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
