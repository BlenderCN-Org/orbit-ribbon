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

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>

#include "background.h"
#include "constants.h"
#include "debug.h"
#include "display.h"
#include "except.h"
#include "gameobj.h"
#include "globals.h"
#include "saving.h"
#include "mode.h"
#include "performance.h"
#include "gloo.h"

// Clipping distance for gameplay objects and background objects respectively
const float GAMEPLAY_CLIP_DIST = 50000;
const float SKY_CLIP_DIST = 1e12;

// Field-of-view in degrees
const float FOV = 45;

// How often in ticks to update the performance info string
const unsigned int MAX_PERF_INFO_AGE = 2000;

GLfloat fade_r, fade_g, fade_b, fade_a;
bool fade_flag = false;

SDL_Surface* screen;

GLsizei Display::screen_width = 1280;
GLsizei Display::screen_height = 800;
GLint Display::screen_depth = 16;
GLfloat Display::screen_ratio;

void Display::set_fade_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
	fade_r = r;
	fade_g = g;
	fade_b = b;
	fade_a = a;
}

void Display::set_fade(bool flag) {
	fade_flag = flag;
}

void Display::init() {
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	if (Saving::get().config().vSync().get()) {
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	}
	std::string win_title = std::string("Orbit Ribbon (") + APP_VERSION + std::string(")");
	SDL_WM_SetCaption(win_title.c_str(), win_title.c_str());
	//FIXME : Set window icon here
	
	GLint videoFlags;
	videoFlags  = SDL_OPENGL;
	videoFlags |= SDL_GL_DOUBLEBUFFER;
	videoFlags |= SDL_HWPALETTE;
	videoFlags |= SDL_SWSURFACE;
	videoFlags |= (Saving::get().config().fullScreen().get() ? SDL_FULLSCREEN : 0);
	screen = SDL_SetVideoMode(get_screen_width(), get_screen_height(), get_screen_depth(), videoFlags);
	if (!screen) {
		throw GameException(std::string("Video mode set failed: ") + std::string(SDL_GetError()));
	}
	
	SDL_ShowCursor(SDL_DISABLE);
	
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
	
	glEnable(GL_TEXTURE_2D);
	
	glEnable(GL_LIGHT1); glLightfv(GL_LIGHT1, GL_DIFFUSE, T3_LIGHT_DIFFUSE);
	// glEnable(GL_LIGHT2); glLightfv(GL_LIGHT1, GL_DIFFUSE, VOY_LIGHT_DIFFUSE); // Enable this once Voy lighting is done by Background
	glEnable(GL_LIGHT3); glLightfv(GL_LIGHT3, GL_DIFFUSE, AMB_LIGHT_DIFFUSE);
	glEnable(GL_LIGHT4); glLightfv(GL_LIGHT4, GL_DIFFUSE, AMB_LIGHT_DIFFUSE);
	glEnable(GL_LIGHT5); glLightfv(GL_LIGHT5, GL_DIFFUSE, AMB_LIGHT_DIFFUSE);
	glEnable(GL_LIGHT6); glLightfv(GL_LIGHT6, GL_DIFFUSE, AMB_LIGHT_DIFFUSE);
	
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
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

void Display::draw_frame() {
	static std::string perf_info;
	static unsigned int last_perf_info = 0; // Tick time at which we last updated perf_info
	
	// 3D drawing mode (projection matrix will be set below)
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	
	// Clear the screen
	Globals::bg->set_clear_color();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Set camera position and orientation
	Globals::mode->set_camera();
	
	// Projection mode for distant objects
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(FOV, screen_ratio, 0.1, SKY_CLIP_DIST);
	glMatrixMode(GL_MODELVIEW);
	
	// Draw all the background objects
	Globals::bg->draw();
	
	// Projection mode for nearby objects
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(FOV, screen_ratio, 0.1, GAMEPLAY_CLIP_DIST);
	glMatrixMode(GL_MODELVIEW);
	
	// Draw every game object (FIXME Do near/far sorting)
	for (GOMap::iterator i = Globals::gameobjs.begin(); i != Globals::gameobjs.end(); ++i) {
		i->second->draw(true);
	}
	
	// 2D drawing mode
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, screen_width, screen_height, 0.0); // Set origin at top-left of screen
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// If fading is enabled, then mask what's been drawn with a big ol' translucent quad
	// FIXME This should be the game mode's responsibility if it wants to do this
	if (fade_flag) {
		glColor4f(fade_r, fade_g, fade_b, fade_a);
		glBegin(GL_QUADS);
			glVertex2f(0, 0);
			glVertex2f(screen_width, 0);
			glVertex2f(screen_width, screen_height);
			glVertex2f(0, screen_height);
		glEnd();
	}
	
	if (Saving::get().config().showFps().get() && SDL_GetTicks() - last_perf_info >= MAX_PERF_INFO_AGE) {
		last_perf_info = SDL_GetTicks();
		perf_info = Performance::get_perf_info() + " " + GLOOBufferedMesh::get_usage_info();
	}
	Globals::sys_font->draw(Point(20, 20), 15, perf_info);
	
	// Output and flip buffers
	SDL_GL_SwapBuffers();
}
