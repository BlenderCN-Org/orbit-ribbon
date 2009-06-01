from __future__ import division

import ode, sys, math, pygame
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *
from OpenGL.GLUT import *

import collision, util, console, resman, camera, joy, sky, titlescreen
from geometry import *

MODE_GAMEPLAY, MODE_TITLE_SCREEN = range(2)

SKY_CLIP_DIST = 1e12
GAMEPLAY_CLIP_DIST = 5000
FOV = 45

#The ODE simulation
#Objects in static_space do not collide with one another.
#Objects in dyn_space collide with those in static_space, as well as with each other.
odeworld = None
static_space = None
dyn_space = None

#A list of all the various game objects
objects = None

#An instance of mission.MissionControl defining the mission parameters.
#FIXME: Currently, must be set externally. Will eventually become responsibility of level loader.
mission_control = None

#An instance of sky.SkyStuff with defining the location of Voy and other distant objects
#FIXME: Currently, must be set externally. Will eventually become responsibility of level loader.
sky_stuff = None

#The current game mode, which determines the virtual interface and the meanining of player input
mode = None

#Input events (from PyGame) which occurred this step
events = []

#The result of calls to joy.getAxes and joy.getButtons at the beginning of this simstep
axes = {}
buttons = {}

#Dictionary of collisions between geoms logged this step
#Each key is the id of an ODE geom
#Value is an array of ODE geoms (the geoms themselves, not ids) that the key geom collided with
#The collision is added both ways, so that if A and B collide, B is in A's list, and A is in B's list too
collisions = {}

#A group for momentary joints; the group is emptied each step, so joints only last for one step
contactgroup = ode.JointGroup()

winsize = (800, 600) #Size of the display window in pixels; TODO: should be a user setting
maxfps = 60 #Max frames per second, and absolute sim-steps per second

fade_color = None #If not None, this color is drawn as a rect covering the entire screen. Useful for fade effects.

player_camera = None #A Camera object describing our 3D viewpoint

screen = None #The PyGame screen

clock = None #An instance of pygame.time.Clock() used for timing; use step count, not this for game-logic timing
totalsteps = 0L #Number of simulation steps we've ran

cons = None #An instances of console.Console used for in-game debugging
watchers = [] #A sequence of console.Watchers used for in-game debugging

title_screen_manager = None #An instance of titlescreen.TitleScreenManager, which handles the behavior of pre-gameplay menus

class QuitException:
	"""Raised when something wants the main loop to end."""
	pass


def ui_init():
	global screen, clock, cons, watchers

	pygame.display.init()
	pygame.display.set_caption('Orbit Ribbon')
	pygame.display.set_icon(pygame.image.load(os.path.join('images', 'logo.png')))
	pygame.mouse.set_visible(0)
	screen = pygame.display.set_mode(winsize, DOUBLEBUF | OPENGL)

	pygame.mixer.init(22050, -16, 2, 512)

	clock = pygame.time.Clock()
	
	joy.init()
	
	glutInit(sys.argv) # GLUT is only used for drawing text and basic geometrical objects, not its full rigamarole of app control
	
	glDepthFunc(GL_LEQUAL)
	
	glClearColor(0, 0, 0, 0)
	glClearDepth(1.0)
	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST)
	
	glEnable(GL_BLEND)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
	
	glColorMaterial(GL_FRONT, GL_DIFFUSE)
	glEnable(GL_COLOR_MATERIAL)
	
	cons = console.Console()
	watchers = []
	sys.stderr = cons.pseudofile
	sys.stdout = cons.pseudofile
	watchers.append(console.Watcher("q", "a", pygame.Rect(20, winsize[1]/5-30, winsize[0]/6, winsize[1]/5-20)))
	watchers.append(console.Watcher("d", "c", pygame.Rect(20, 2*winsize[1]/5-30, winsize[0]/6, winsize[1]/5-20)))
	watchers.append(console.Watcher("p", "l", pygame.Rect(5*winsize[0]/6 - 20, winsize[1]/5-30, winsize[0]/6, winsize[1]/5-20)))
	watchers.append(console.Watcher("j", "n", pygame.Rect(5*winsize[0]/6 - 20, 2*winsize[1]/5-30, winsize[0]/6, winsize[1]/5-20)))


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
	"""Initializes the camera and simulation, including ODE.
	
	You must call this before calling run().
	"""
	
	global odeworld, static_space, dyn_space, objects, totalsteps, player_camera, title_screen_manager, fade_color
	totalsteps = 0L
	odeworld = ode.World()
	odeworld.setQuickStepNumIterations(10)
	static_space = ode.HashSpace()
	dyn_space = ode.HashSpace()
	objects = []
	title_screen_manager = titlescreen.TitleScreenManager()
	set_game_mode(MODE_TITLE_SCREEN)
	fade_color = None


def set_game_mode(new_mode):
	global mode
	mode = new_mode
	
	global player_camera
	if mode == MODE_GAMEPLAY:
		player_camera = camera.FollowCamera(
			target_obj = objects[0]
		)
	elif mode == MODE_TITLE_SCREEN:
		player_camera = title_screen_manager.camera


def sim_deinit():
	"""Deinitializes the camera and simulation, including ODE.
	
	You can call this and then call sim_init() again to forcibly clear the game state.
	Other than that, you don't need to call this.
	"""
	
	global odeworld, static_space, dyn_space, objects, totalsteps, player_camera, mode, title_screen_manager, fade_color
	totalsteps = 0L
	odeworld = None
	static_space = None
	dyn_space = None
	objects = None
	ode.CloseODE()
	player_camera = None
	mode = None
	title_screen_manager = None
	fade_color = None


def _sim_step():
	"""Runs one step of the simulation. This is (1/maxfps)th of a simulated second."""
	
	global collisions, contactgroup
	
	#Update mission status
	mission_control.step()
	
	#Calculate collisions, run ODE simulation
	contactgroup.empty()
	collisions = {}
	dyn_space.collide(contactgroup, collision.collision_cb) #Collisions among dyn_space objects
	ode.collide2(dyn_space, static_space, contactgroup, collision.collision_cb) #Colls between dyn_space objects and static_space objs
	odeworld.quickStep(1/maxfps)
	
	#Load each GameObj's state with the new information ODE calculated
	for o in objects:
		o.sync_ode()
	
	#Have each object do any simulation stuff it needs, then damp its linear and angular velocity to simulate air friction
	for o in objects:
		o.step()
		o.damp()


def _draw_frame():
	# Reset state
	glMatrixMode(GL_MODELVIEW)
	glLoadIdentity()
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
	
	# Position the camera
	gluLookAt(*player_camera.get_camvals())
	
	# 3D projection mode for sky objects without depth-testing
	glDisable(GL_DEPTH_TEST)
	glDisable(GL_LIGHTING)
	glMatrixMode(GL_PROJECTION)
	glLoadIdentity()
	gluPerspective(FOV, winsize[0]/winsize[1], 0.1, SKY_CLIP_DIST)
	glMatrixMode(GL_MODELVIEW)
	
	# Draw the sky objects
	sky_stuff.draw()
	
	# 3D projection mode for gameplay objects with depth-testing
	glEnable(GL_DEPTH_TEST)
	glEnable(GL_LIGHTING)
	glMatrixMode(GL_PROJECTION)
	glLoadIdentity()
	gluPerspective(FOV, winsize[0]/winsize[1], 0.1, GAMEPLAY_CLIP_DIST)
	glMatrixMode(GL_MODELVIEW)
	
	# Draw all objects in the list
	for o in objects:
		o.draw()
	
	# 2D drawing mode
	glMatrixMode(GL_PROJECTION)
	glLoadIdentity()
	gluOrtho2D(0.0, winsize[0], winsize[1], 0.0)
	glMatrixMode(GL_MODELVIEW)
	glLoadIdentity()
	glDisable(GL_DEPTH_TEST)
	glDisable(GL_LIGHTING)
	
	# Draw the virtual interface
	if mode == MODE_GAMEPLAY:
		mission_control.draw()
	else:
		title_screen_manager.draw()
	
	# If we have a fade color, apply it
	if fade_color is not None:
		glColor4f(*fade_color)
		glBegin(GL_QUADS)
		glVertex2f(0,          0)
		glVertex2f(winsize[0], 0)
		glVertex2f(winsize[0], winsize[1])
		glVertex2f(0,          winsize[1])
		glEnd()
	
	# Draw the watchers
	for w in watchers:
		if w.expr != None:
			w.draw()
	
	# Draw the console, if it's up
	cons.draw()
	
	# Output and flip buffers
	glFlush()
	pygame.display.flip()


def _proc_input():
	global events
	events = []
	for event in pygame.event.get():
		cons.handle(event)
		if event.type == pygame.QUIT:
			raise QuitException
		elif event.type == pygame.KEYDOWN and event.key == pygame.K_F4:
			raise QuitException
		else:
			if not cons.active:
				events.append(event)
	
	# Update the watchers
	for w in watchers:
		if w.expr != None:
			w.update()
	
	global axes, buttons
	axes = joy.getAxes()
	buttons = joy.getButtons()


def run():
	"""Runs the game.
	
	You have to call ui_init() and sim_init() before running this.
	"""
	global totalsteps
	
	try:
		totalms = 0L #Total number of milliseconds passed
		while True:
			elapsedms = clock.tick(maxfps)
			
			if cons.active:
				_proc_input()
			elif mode == MODE_GAMEPLAY:
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
					totalsteps += 1
			elif mode == MODE_TITLE_SCREEN:
				_proc_input()
			
			#Draw everything
			_draw_frame()
	except QuitException:
		pass
