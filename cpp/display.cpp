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

#include "constants.h"
#include "debug.h"
#include "display.h"
#include "except.h"
#include "gloo.h"

const GLfloat gameplay_clip_dist = 50000;
const GLfloat sky_clip_dist = 1e12;
const GLfloat fov = 45;

GLfloat fade_r, fade_g, fade_b, fade_a;
bool fade_flag = false;

SDL_Surface* screen;

void Display::set_fade_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
	fade_r = r;
	fade_g = g;
	fade_b = b;
	fade_a = a;
}

void Display::set_fade(bool flag) {
	fade_flag = flag;
}

GLsizei Display::get_screen_width() {
	return 800;
}

GLsizei Display::get_screen_height() {
	return 600;
}

GLint Display::get_screen_depth() {
	return 16;
}

boost::shared_ptr<GLOOTexture> dbg_texture; // FIXME

void Display::_init() {
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
	   throw GameException(std::string("Video initialization failed: ") + std::string(SDL_GetError()));
	}
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	std::string win_title = std::string("Orbit Ribbon (") + APP_VERSION + std::string(")");
	SDL_WM_SetCaption(win_title.c_str(), win_title.c_str());
	//FIXME : Set window icon here
	
	GLint videoFlags;
	videoFlags  = SDL_OPENGL;
	videoFlags |= SDL_GL_DOUBLEBUFFER;
	videoFlags |= SDL_HWPALETTE;
	videoFlags |= SDL_SWSURFACE;
	screen = SDL_SetVideoMode(get_screen_width(), get_screen_height(), get_screen_depth(), videoFlags);
	if (!screen) {
		throw GameException(std::string("Video mode set failed: ") + std::string(SDL_GetError()));
	}
	
	GLenum glew_err = glewInit();
	if (glew_err != GLEW_OK) {
		throw GameException(std::string("GLEW init failed: " ) + std::string((const char*)glewGetErrorString((glew_err))));
	}
	
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	_screen_resize();
	
	dbg_texture = GLOOTexture::create("jungletex.png"); // FIXME
}

void Display::_screen_resize() {
	glViewport(0, 0, Display::get_screen_width(), Display::get_screen_height());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}

void Display::_draw_frame() {
	const GLsizei screen_width = Display::get_screen_width();
	const GLsizei screen_height = Display::get_screen_height();
	const GLfloat screen_ratio = GLfloat(screen_width)/GLfloat(screen_height);
	
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// 3D projection mode for sky objects and billboards without depth-testing
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, screen_ratio, 0.1, sky_clip_dist);
	glMatrixMode(GL_MODELVIEW);

	
	// FIXME Test render
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -6.0f);
	glScalef(0.05, 0.05, 0.05);
	GLint row, col;
	for (row = 0; row < 30; ++row) {
		for (col = 0; col < 40; ++col) {
			glPushMatrix();
			glTranslatef(-60 + col*3, 45 - row*3, 0);
			glPushMatrix();
			glTranslatef(0.0f, 0.0f, -3.0f);
			glColor3f( 0.5f, 0.5f, 1.0f);
			glBegin( GL_QUADS );				 /* Draw A Quad			  */
				glVertex3f(  1.0f,  1.0f,  0.0f ); /* Top Right Of The Quad	*/
				glVertex3f( -1.0f,  1.0f,  0.0f ); /* Top Left Of The Quad	 */
				glVertex3f( -1.0f, -1.0f,  0.0f ); /* Bottom Left Of The Quad  */
				glVertex3f(  1.0f, -1.0f,  0.0f ); /* Bottom Right Of The Quad */
			glEnd();							/* Done Drawing The Quad	*/
			glPopMatrix();
			glPushMatrix();
			glBegin( GL_TRIANGLES );			 /* Drawing Using Triangles	   */
				glColor3f(   1.0f,  0.0f,  0.0f ); /* Red						   */
				glVertex3f(  0.0f,  1.0f,  0.0f ); /* Top Of Triangle			   */
				glColor3f(   0.0f,  1.0f,  0.0f ); /* Green						 */
				glVertex3f( -1.0f, -1.0f,  0.0f ); /* Left Of Triangle			  */
				glColor3f(   0.0f,  0.0f,  1.0f ); /* Blue						  */
				glVertex3f(  1.0f, -1.0f,  0.0f ); /* Right Of Triangle			 */
			glEnd();							/* Finished Drawing The Triangle */
			glPopMatrix();
			glPopMatrix();
		}
	}
	glPopMatrix();
	
	// 3D projection mode for nearby gameplay objects with depth-testing
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, screen_ratio, 0.1, gameplay_clip_dist);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// 2D drawing mode
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, screen_width, screen_height, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// If fading is enabled, then mask what's been drawn with a big ol' translucent quad
	if (fade_flag) {
		glColor4f(fade_r, fade_g, fade_b, fade_a);
		glBegin(GL_QUADS);
			glVertex2f(0, 0);
			glVertex2f(screen_width, 0);
			glVertex2f(screen_width, screen_height);
			glVertex2f(0, screen_height);
		glEnd();
	}
	
	// Output and flip buffers
	SDL_GL_SwapBuffers();
}
