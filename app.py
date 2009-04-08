from __future__ import division
import ode, sys, math, pygame
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

import collision, util, console, resman, app
from geometry import *

#The ODE simulation
#Objects in static_space do not collide with one another.
#Objects in dyn_space collide with those in static_space, as well as with each other.
odeworld = None
static_space = None
dyn_space = None

#A list of all the various game objects
objects = None

#Input events (from PyGame) which occurred this step
events = []

#Dictionary of collisions between geoms logged this step
#Each key is the id of an ODE geom
#Value is an array of ODE geoms (the geoms themselves, not ids) that the key geom collided with
#The collision is added both ways, so that if A and B collide, B is in A's list, and A is in B's list too
collisions = {}

#A group for momentary joints; the group is emptied each step, so joints only last for one step
contactgroup = ode.JointGroup()

winsize = (800, 600) #Size of the display window in pixels; TODO: should be a user setting
maxfps = 60 #Max frames per second, and absolute sim-steps per second
camera = Point() #Where, in game meters, the view is centered
zoom = 1.0 #The zoom factor for the camera (1.0 is neutral)
screen = None #The PyGame screen
clock = None #An instance of pygame.time.Clock() used for timing; use step count, not this for game-logic timing
totalsteps = 0L #Number of simulation steps we've ran
cons = None #An instances of console.Console used for in-game debugging
watchers = [] #A sequence of console.Watchers used for in-game debugging

class QuitException:
	"""Raised when something wants the main loop to end."""
	pass

def ui_init():
	global screen, clock, cons, watchers

	pygame.init()
	pygame.display.set_caption('Orbit Ribbon')
	pygame.mouse.set_visible(0)
	screen = pygame.display.set_mode(winsize, DOUBLEBUF | OPENGL)
	clock = pygame.time.Clock()
	
	glutInit(sys.argv) # GLUT is only used for drawing text
	
	glViewport(0, 0, winsize[0], winsize[1])
	glMatrixMode(GL_PROJECTION)
	glLoadIdentity()
	gluPerspective(45, 1.0*(winsize[0]/winsize[1]), 0.1, 100.0)
	glMatrixMode(GL_MODELVIEW)
	glLoadIdentity()
	
	glEnable(GL_DEPTH_TEST)
	glDepthFunc(GL_LEQUAL)
	
	glClearColor(0.8, 0.8, 1.0, 0.0)
	glClearDepth(1.0)
	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST)
	
	glEnable(GL_BLEND)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
	glShadeModel(GL_SMOOTH)
	glEnable(GL_POINT_SMOOTH)
	glEnable(GL_LINE_SMOOTH)
	glEnable(GL_POLYGON_SMOOTH)

	LightAmbient = (0.5, 0.5, 0.5, 1.0)
	LightDiffuse = (1.0, 1.0, 1.0, 1.0)
	LightPosition = (0.0, 10.0, 2.0, 1.0)
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient)
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse)
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition)
	glEnable(GL_LIGHT1)
	glEnable(GL_LIGHTING)
	glColorMaterial(GL_FRONT, GL_DIFFUSE)
	glEnable(GL_COLOR_MATERIAL)
	
	glPointSize(4)
	glLineWidth(2)
	
	cons = console.Console()
	watchers = []
	sys.stderr = cons.pseudofile
	sys.stdout = cons.pseudofile
	watchers.append(console.Watcher(pygame.Rect(20, winsize[1]/3-30, winsize[0]/4, winsize[1]/3-20)))
	watchers.append(console.Watcher(pygame.Rect(20, 2*winsize[1]/3-30, winsize[0]/4, winsize[1]/3-20)))
	watchers.append(console.Watcher(pygame.Rect(3*winsize[0]/4 - 20, winsize[1]/3-30, winsize[0]/4, winsize[1]/3-20)))
	watchers.append(console.Watcher(pygame.Rect(3*winsize[0]/4 - 20, 2*winsize[1]/3-30, winsize[0]/4, winsize[1]/3-20)))

def ui_deinit():
	global screen, clock, cons, watchers
	
	resman.unload_all()
	pygame.quit()
	
	screen = None
	clock = None
	cons = None
	watchers = []
	
	sys.stderr = sys.__stderr__
	sys.stdout = sys.__stdout__

def sim_init():
	"""Initializes the simulation, including ODE.
	
	You must call this before calling run().
	"""
	
	global odeworld, static_space, dyn_space, objects, totalsteps
	totalsteps = 0L
	odeworld = ode.World()
	odeworld.setQuickStepNumIterations(10)
	static_space = ode.HashSpace()
	dyn_space = ode.HashSpace()
	objects = []

def sim_deinit():
	"""Deinitializes the camera and simulation, including ODE.
	
	You can call this and then call sim_init() again to forcibly clear the game state.
	Other than that, you don't need to call this.
	"""

	global odeworld, static_space, dyn_space, objects
	odeworld = None
	static_space = None
	dyn_space = None
	objects = None
	ode.CloseODE()

def _sim_step():
	"""Runs one step of the simulation. This is (1/maxfps)th of a simulated second."""
	
	global collisions, contactgroup
	
	#Calculate collisions, run ODE simulation
	contactgroup.empty()
	collisions = {}
	dyn_space.collide(contactgroup, collision.collision_cb) #Collisions among dyn_space objects
	ode.collide2(dyn_space, static_space, contactgroup, collision.collision_cb) #Colls between dyn_space objects and static_space objs
	odeworld.quickStep(1/maxfps)
		
	#Load each GameObj's state with the new information ODE calculated
	for o in objects:
		o.sync_ode()
	
	#Have each object do any simulation stuff it needs
	for o in objects:
		o.step()

def _draw_frame():
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
	glLoadIdentity();
	# FIXME: Incorporate zoom
	
	#Draw all the objects
	glPushMatrix()
	for o in objects:
		o.draw()
	glPopMatrix()
	
	for w in watchers:
		if w.expr != None:
			w.update()
			w.draw()
	
	glLoadIdentity()
	cons.draw()
	
	glFlush()
	pygame.display.flip()

def _proc_input():
	global events
	events = []
	for event in pygame.event.get():
		cons.handle(event)
		if event.type == pygame.QUIT:
			raise QuitException
		elif event.type == pygame.KEYDOWN and event.key == pygame.K_BACKSPACE:
			raise QuitException
		else:
			events.append(event)

def run():
	"""Runs the game.
	
	You have to call ui_init() and sim_init() before running this.
	"""
	global totalsteps
	
	try:
		totalms = 0L #Total number of milliseconds passed
		while True:
			elapsedms = clock.tick(maxfps)
			
			if not cons.active:
				totalms += elapsedms
				
				#Figure out how many simulation steps we're doing this frame.
				#In theory, shouldn't be zero, since max frames per second is the same as steps per second
				#However, it's alright to be occasionally zero, since clock.tick is sometimes slightly off
				#FIXME: Do we really need totalms?
				steps = int(math.floor((totalms*maxfps/1000)))-totalsteps
				
				#Run the simulation the desired number of steps
				for i in range(steps):
					_proc_input()
					_sim_step()
				
				totalsteps += steps
			else:
				_proc_input()
			
			#Draw everything
			_draw_frame()
	except QuitException:
		pass
