#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>

#include <string>
#include <list>

#include "app.h"
#include "except.h"
#include "sim.h"

const GLfloat gameplay_clip_dist = 50000;
const GLfloat sky_clip_dist = 1e12;
const GLfloat fov = 45;

SDL_Surface* screen;

GLfloat fade_r, fade_g, fade_b, fade_a;
bool fade_flag = false;

GLint total_steps = 0;

std::list<SDL_Event> frame_events;

void screen_resize() {
	glViewport(0, 0, App::get_screen_width(), App::get_screen_height());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}

void draw_frame() {
	const GLsizei screen_width = App::get_screen_width();
	const GLsizei screen_height = App::get_screen_height();
	
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// 3D projection mode for nearby gameplay objects with depth-testing
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, GLfloat(screen_width)/GLfloat(screen_height), 0.1, gameplay_clip_dist);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
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
	
	// 2D drawing mode
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, screen_width, screen_height, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	
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

void App::init() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
	   throw GameException(std::string("Video initialization failed: ") + std::string(SDL_GetError()));
	}
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	GLint videoFlags;
	videoFlags  = SDL_OPENGL;
	videoFlags |= SDL_GL_DOUBLEBUFFER;
	videoFlags |= SDL_HWPALETTE;
	videoFlags |= SDL_SWSURFACE;
	screen = SDL_SetVideoMode(get_screen_width(), get_screen_height(), get_screen_depth(), videoFlags);
	if (!screen) {
		throw GameException(std::string("Video mode set failed: ") + std::string(SDL_GetError()));
	}
	
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	Sim::_init();
	
	screen_resize();
}

void App::load_area(const std::string& area_name) {
}

void App::load_mission(const std::string& mission_name) {
}

void App::run() {
	const GLint max_ticks_per_frame = 1000/get_max_fps();
	int unsimulated_ticks = 0;
	
	while (1) {
		GLint frame_start = SDL_GetTicks();
		
		// Add all new events to the events list for this frame, and quit on QUIT events
		frame_events.clear();
		SDL_Event event;
	   while (SDL_PollEvent(&event)) {
			frame_events.push_back(event);
		  	if (event.type == SDL_QUIT) {
				throw GameQuitException("Closed on quit event");
			}
		}
		
		// Do simulation steps until we've caught up with the display
		while (unsimulated_ticks > max_ticks_per_frame) {
			Sim::_sim_step();
			total_steps += 1;
			unsimulated_ticks -= max_ticks_per_frame;
		}
		
		// Re-draw the display
		draw_frame();
		
		// Sleep if we're running faster than our maximum fps
		GLint frame_ticks = SDL_GetTicks() - frame_start;
		if (frame_ticks > 0 & frame_ticks < max_ticks_per_frame) {
			SDL_Delay(max_ticks_per_frame - frame_ticks); // Slow down, buster!
		}
		
		// The time that passed during this frame needs to pass in the simulator next frame
		unsimulated_ticks += SDL_GetTicks() - frame_start;
	}
}

const std::list<SDL_Event>& App::get_frame_events() {
	return frame_events;
}

void App::set_fade_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
	fade_r = r;
	fade_g = g;
	fade_b = b;
	fade_a = a;
}

void App::set_fade(bool flag) {
	fade_flag = flag;
}

GLsizei App::get_screen_width() {
	return 800;
}

GLsizei App::get_screen_height() {
	return 600;
}

GLint App::get_screen_depth() {
	return 16;
}

GLint App::get_total_steps() {
	return 16;
}
