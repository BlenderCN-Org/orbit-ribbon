/* Based upon NeHe SDL lesson 04, which had this header: */
/*
 * This code was created by Jeff Molofee '99 
 * (ported to Linux/SDL by Ti Leggett '01)
 *
 * If you've found this code useful, please let me know.
 *
 * Visit Jeff at http://nehe.gamedev.net/
 * 
 * or for port-specific comments, questions, bugreports etc. 
 * email to leggett@eecs.tulane.edu
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"

/* screen width, height, and bit depth */
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600
#define SCREEN_BPP	 16

/* Define our booleans */
#define TRUE  1
#define FALSE 0

/* This is our SDL surface */
SDL_Surface *surface;

/* function to release/destroy our resources and restoring the old desktop */
void Quit( int returnCode ) {
	/* clean up the window */
	SDL_Quit();

	/* and exit appropriately */
	exit( returnCode );
}

/* function to reset our viewport after a window resize */
int resizeWindow( int width, int height ) {
	/* Height / width ration */
	GLfloat ratio;
	
	/* Protect against a divide by zero */
	if ( height == 0 )
	height = 1;
	
	ratio = ( GLfloat )width / ( GLfloat )height;
	
	/* Setup our viewport. */
	glViewport( 0, 0, ( GLsizei )width, ( GLsizei )height );
	
	/* change to the projection matrix and set our viewing volume. */
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	/* Set our perspective */
	gluPerspective( 45.0f, ratio, 0.1f, 100.0f );
	
	/* Make sure we're chaning the model view and not the projection */
	glMatrixMode( GL_MODELVIEW );
	
	/* Reset The View */
	glLoadIdentity();
	
	return( TRUE );
}

/* general OpenGL initialization function */
int initGL( GLvoid ) {

	/* Enable smooth shading */
	glShadeModel( GL_SMOOTH );

	/* Set the background black */
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

	/* Depth buffer setup */
	glClearDepth( 1.0f );

	/* Enables Depth Testing */
	glEnable( GL_DEPTH_TEST );

	/* The Type Of Depth Test To Do */
	glDepthFunc( GL_LEQUAL );

	/* Really Nice Perspective Calculations */
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

	return( TRUE );
}

/* Here goes our drawing code */
int drawGLScene( GLvoid ) {
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
	
	return( TRUE );
}

int main( int argc, char **argv ) {
	/* Flags to pass to SDL_SetVideoMode */
	int videoFlags;
	/* main loop variable */
	int done = FALSE;
	/* used to collect events */
	SDL_Event event;
	/* this holds some info about our display */
	const SDL_VideoInfo *videoInfo;

	/* initialize SDL */
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
	   fprintf( stderr, "Video initialization failed: %s\n", SDL_GetError() );
	   Quit( 1 );
	}

	/* the flags to pass to SDL_SetVideoMode */
	videoFlags  = SDL_OPENGL;		  /* Enable OpenGL in SDL */
	videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
	videoFlags |= SDL_HWPALETTE;	   /* Store the palette in hardware */
	videoFlags |= SDL_SWSURFACE;
	
	/* Sets up OpenGL double buffering */
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	/* get a SDL surface */
	surface = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,
				videoFlags );

	/* Verify there is a surface */
	if ( !surface ) {
		fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError() );
		Quit( 1 );
	}

	/* initialize OpenGL */
	initGL();

	/* resize the initial window */
	resizeWindow( SCREEN_WIDTH, SCREEN_HEIGHT );
	
	/* These are to calculate our fps */
	GLint frames = 0;
	GLint t0	 = 0;
	t0 = SDL_GetTicks();

	/* wait for events */ 
	while ( !done ) {
	   /* handle the events in the queue */

	   while ( SDL_PollEvent( &event ) ) {
		   switch( event.type ) {
				case SDL_QUIT:
					/* handle quit requests */
					done = TRUE;
					break;
				default:
					break;
			}
		}

	   /* draw the scene */
		drawGLScene();
	
		/* Gather our frames per second */
		++frames;
		GLint t = SDL_GetTicks();
		if (t - t0 >= 10000) {
			GLfloat seconds = (t - t0) / 1000.0;
			GLfloat fps = frames / seconds;
			printf("%d frames in %g seconds = %g FPS\n", frames, seconds, fps);
			done = TRUE;
		}
	}

	/* clean ourselves up and exit */
	Quit(0);

	/* Should never get here */
	return(0);
}
