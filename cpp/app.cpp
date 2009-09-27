#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <ode/ode.h>

#include <string>
#include <iostream>

#include "app.h"
#include "except.h"

SDL_Surface* App::_screen;

void App::init() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
	   throw GameException(std::string("Video initialization failed: ") + std::string(SDL_GetError()));
	}
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	int videoFlags;
	videoFlags  = SDL_OPENGL;
	videoFlags |= SDL_GL_DOUBLEBUFFER;
	videoFlags |= SDL_HWPALETTE;
	videoFlags |= SDL_SWSURFACE;
	_screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, videoFlags);
	if (!_screen) {
		throw GameException(std::string("Video mode set failed: ") + std::string(SDL_GetError()));
	}
	
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glViewport(0, 0, (GLsizei)SCREEN_WIDTH, (GLsizei)SCREEN_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, GLfloat(SCREEN_WIDTH)/GLfloat(SCREEN_HEIGHT), 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void App::load_area(const std::string& area_name) {
}

void App::load_mission(const std::string& mission_name) {
}

void App::run() {
	GLint t0 = SDL_GetTicks();

	while (1) {
		SDL_Event event;
	   while (SDL_PollEvent(&event)) {
		   switch(event.type) {
				case SDL_QUIT:
					break;
				default:
					break;
			}
		}
		
		_draw_frame();
	
		/* Gather our frames per second */
		GLint t = SDL_GetTicks();
		if (t - t0 >= 10000) {
			std::cout << "Finished" << std::endl;
			break;
		}
	}
}

void App::_sim_step() {
}

void App::_draw_frame() {
	/* rotational vars for the triangle and quad, respectively */
	static GLfloat rtri, rquad;
	
	GLint t = SDL_GetTicks();
	
	/* Clear The Screen And The Depth Buffer */
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -6.0f);
	glScalef(0.05, 0.05, 0.05);
	
	int row, col;
	for (row = 0; row < 30; ++row) {
		for (col = 0; col < 40; ++col) {
			glPushMatrix();
			glTranslatef(-60 + col*3, 45 - row*3, 0);
			
			glPushMatrix();
			glRotatef( rtri, 0.0f, 1.0f, 0.0f );
			glBegin( GL_TRIANGLES );			 /* Drawing Using Triangles	   */
				glColor3f(   1.0f,  0.0f,  0.0f ); /* Red						   */
				glVertex3f(  0.0f,  1.0f,  0.0f ); /* Top Of Triangle			   */
				glColor3f(   0.0f,  1.0f,  0.0f ); /* Green						 */
				glVertex3f( -1.0f, -1.0f,  0.0f ); /* Left Of Triangle			  */
				glColor3f(   0.0f,  0.0f,  1.0f ); /* Blue						  */
				glVertex3f(  1.0f, -1.0f,  0.0f ); /* Right Of Triangle			 */
			glEnd();							/* Finished Drawing The Triangle */
			glPopMatrix();

			glPushMatrix();
			glTranslatef(0.0f, 0.0f, -3.0f);
			glRotatef( rquad, 1.0f, 0.0f, 0.0f );
			glColor3f( 0.5f, 0.5f, 1.0f);
			glBegin( GL_QUADS );				 /* Draw A Quad			  */
				glVertex3f(  1.0f,  1.0f,  0.0f ); /* Top Right Of The Quad	*/
				glVertex3f( -1.0f,  1.0f,  0.0f ); /* Top Left Of The Quad	 */
				glVertex3f( -1.0f, -1.0f,  0.0f ); /* Bottom Left Of The Quad  */
				glVertex3f(  1.0f, -1.0f,  0.0f ); /* Bottom Right Of The Quad */
			glEnd();							/* Done Drawing The Quad	*/
			glPopMatrix();
			
			glPopMatrix();
		}
	}
	
	glPopMatrix();
	
	/* Draw it to the screen */
	SDL_GL_SwapBuffers();
	
	GLint tdiff = SDL_GetTicks() - t;
	
	/* Increase The Rotation Variable For The Triangle ( NEW ) */
	rtri  += 0.2f*tdiff;
	/* Decrease The Rotation Variable For The Quad	 ( NEW ) */
	rquad -=0.15f*tdiff;
}
