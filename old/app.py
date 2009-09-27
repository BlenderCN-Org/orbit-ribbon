from __future__ import division

import ode, sys, math, pygame, pickle, time
from pygame.locals import *

import collision, console, camera, sky, titlescreen, ore, avatar, inputs
from geometry import *
from gl import *

# The directory where the application is installed
# Py2Exe screws with this by putting the path inside a zip file, so go up if that is the case
APP_DIR = os.path.realpath(sys.path[0])
cutoffPoint = APP_DIR.rfind(os.path.sep + "library.zip")
if cutoffPoint != -1:
   APP_DIR = APP_DIR[:cutoffPoint]

MODE_GAMEPLAY, MODE_TITLE_SCREEN = range(2)

SKY_CLIP_DIST = 1e12
GAMEPLAY_CLIP_DIST = 50000
FOV = 45

VERSION = "prealpha"

#The ODE simulation
#Objects in static_space do not collide with one another.
#Objects in dyn_space collide with those in static_space, as well as with each other.
odeworld = None
static_space = None
dyn_space = None

#The OREManager for currently active ORE file (Orbit Ribbon Export data, from the 3d editor)
ore_man = None

#The currently active OREArea, or None. The tstart variable is the tick time at which cur_area was set by init_area.
cur_area = None
cur_area_tstart = None

#The currently active OREMission, or None. The tstart variable is the tick time at which cur_mission was set by init_mission.
cur_mission = None
cur_mission_tstart = None

#When an area or mission selected, a list of all the various gameplay objects.
objects = None

#When in gameplay, an instance of mission.MissionControl defining the mission parameters.
mission_control = None

#An instance of sky.SkyStuff with defining the location of Voy and other distant objects
sky_stuff = None

#The current game mode, which determines the virtual interface and the meanining of player input
mode = None

#Input events (from PyGame) which occurred this step
events = []

#INTENT_* constants (from the inputs module) triggered by PyGame events this step
event_intents = set()

#Billboard objects which should be drawn this frame
billboards = []

#The input manager used to track what the player does with the controls
input_man = None

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

timings = None # A list of lists, each sublist containing timestamps recorded at various points during a frame, or None if this feature is disabled
timing_names = None # A list names assigned to each column in timings, or None if timing isn't enabled

def _rectime_newframe():
	if timings is not None:
		timings.append([])
		_rectime("newframe")

def _rectime(colname):
	if timings is not None:
		if len(timings) == 1:
			timing_names.append(colname)
		timings[-1].append(time.time())

class QuitException:
	"""Raised when something wants the main loop to end."""
	pass


def ui_init():
	"""Initializes the user interface (window, input devices, etc.)

	Must be called before sim_init().
	"""
	global screen, clock, cons, watchers, input_man
	
	cons = console.Console()
	watchers = []
	sys.stderr = cons.pseudofile
	sys.stdout = cons.pseudofile
	watchers.append(console.Watcher(inputs.INTENT_DEBUG_A_AXIS, pygame.Rect(20, winsize[1]/5-30, winsize[0]/6, winsize[1]/5-20)))
	watchers.append(console.Watcher(inputs.INTENT_DEBUG_B_AXIS, pygame.Rect(20, 2*winsize[1]/5-30, winsize[0]/6, winsize[1]/5-20)))
	watchers.append(console.Watcher(inputs.INTENT_DEBUG_C_AXIS, pygame.Rect(5*winsize[0]/6 - 20, winsize[1]/5-30, winsize[0]/6, winsize[1]/5-20)))
	watchers.append(console.Watcher(inputs.INTENT_DEBUG_D_AXIS, pygame.Rect(5*winsize[0]/6 - 20, 2*winsize[1]/5-30, winsize[0]/6, winsize[1]/5-20)))
	
	print
	print "Orbit Ribbon, Version: %s" % VERSION
	print
	
	pygame.display.init()
	pygame.display.set_caption('Orbit Ribbon')
	pygame.display.set_icon(pygame.image.load(os.path.join(APP_DIR, 'images', 'logo.png')))
	pygame.mouse.set_visible(0)
	print "Setting display mode %s" % str(winsize)
	screen = pygame.display.set_mode(winsize, DOUBLEBUF | OPENGL)
	
	input_man = inputs.InputManager()
	
	pygame.mixer.init(22050, -16, 2, 512)
	
	clock = pygame.time.Clock()
	
	glutInit(sys.argv) # GLUT is only used for drawing text and basic geometrical objects, not its full rigamarole of app control
	
	glDepthFunc(GL_LEQUAL)
	
	glClearColor(0, 0, 0, 0)
	glClearDepth(1.0)
	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST)
	
	cachingGlEnable(GL_BLEND)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
	
	glEnableClientState(GL_VERTEX_ARRAY)
	glEnableClientState(GL_NORMAL_ARRAY)
	glEnableClientState(GL_TEXTURE_COORD_ARRAY)
	

def sim_init(timing = False):
	"""Initializes the camera and simulation, including ODE.
	
	You must call this before calling run().
	
	You may call this in order to reset the game.
	"""
	
	global ore_man, odeworld, static_space, dyn_space, objects, totalsteps, player_camera, title_screen_manager, fade_color, sky_stuff
	global cur_area, cur_area_tstart, cur_mission, cur_mission_tstart
	global timings, timing_names

	if timing:
		timings = []
		timing_names = []
	
	totalsteps = 0L
	odeworld = ode.World()
	odeworld.setQuickStepNumIterations(10)
	static_space = ode.HashSpace()
	dyn_space = ode.HashSpace()
	objects = []
	title_screen_manager = titlescreen.TitleScreenManager()
	player_camera = title_screen_manager.camera
	fade_color = None
	sky_stuff = sky.SkyStuff()
	
	fh = file(os.path.join(APP_DIR, 'orefiles', 'main.ore'))
	ore_man = ore.OREManager(fh)
	
	cur_area = None
	cur_area_tstart = None
	cur_mission = None
	cur_mission_tstart = None


def init_area(areaname):
	"""Loads the given area, by internal name, into the game state. This includes objects, sky, etc."""
	global objects, sky_stuff, cur_area, cur_area_tstart
	cur_area = ore_man.areas[areaname]
	cur_area_tstart = pygame.time.get_ticks()
	objects = cur_area.objects[:] # FIXME Need to copy the contents of the objects themselves, so that the level can be restarted
	sky_stuff = cur_area.sky_stuff # FIXME Should probably also copy the SkyStuff for the sake of completeness
	print "Loaded area %s, object count %u" % (areaname, len(objects))


def init_mission(missionname):
	"""Loads the given mission, by internal name, into the game state. This includes mission objects, objectives, camera, etc.
	
	You must first init the mission's area before calling this."""
	global objects, mission_control, player_camera, cur_mission, cur_mission_tstart
	cur_mission = cur_area.missions[missionname]
	cur_mission_tstart = pygame.time.get_ticks()
	objects.extend(cur_mission.objects[:]) # FIXME Need to copy the contents of the objects themselves, so that the level can be restarted
	mission_control = cur_mission.mission_control # FIXME Should probably also copy the MissionControl for the sake of completeness
	
	# Find the Avatar object, then set the camera to follow it
	avatar_obj = None
	for obj in objects:
		if isinstance(obj, avatar.Avatar):
			avatar_obj = obj
			break
	if avatar_obj is None:
		raise RuntimeError("Unable to find Avatar in mission %s!" % missionname)
	player_camera = camera.FollowCamera(target_obj = avatar_obj)
	
	print "Loaded mission %s, object count %u" % (missionname, len(objects))


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
	_rectime("DF-Reset")
	
	# Locate the camera position, and orient to it
	camvals = player_camera.get_camvals()
	campos = player_camera.get_position()
	gluLookAt(*camvals)
	
	# Sort objects to be drawn into those which are far and need to be drawn as billboards, and those which are close and can be drawn as objects
	near_objs, far_objs = [], [] # Lists of GameObjs
	thresh = GAMEPLAY_CLIP_DIST * 0.9
	for o in objects:
		if o.pos.dist_to(campos) > thresh:
			far_objs.append(o)
		else:
			near_objs.append(o)
	_rectime("DF-Sort")
	
	# 3D projection mode for sky objects and billboards without depth-testing
	cachingGlDisable(GL_LIGHTING)
	glMatrixMode(GL_PROJECTION)
	glLoadIdentity()
	gluPerspective(FOV, winsize[0]/winsize[1], 0.1, SKY_CLIP_DIST)
	glMatrixMode(GL_MODELVIEW)
	
	# Draw the atmosphere
	cachingGlDisable(GL_DEPTH_TEST)
	sky_stuff.draw_geometry()
	_rectime("DF-Atmo")
	
	# Draw the billboards for sky objects and distant gameplay objects
	cachingGlEnable(GL_DEPTH_TEST)
	cachingGlEnable(GL_TEXTURE_2D)
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE)
	sky_stuff.draw_billboards()
	for o in far_objs:
		o.distdraw()
	cachingGlDisable(GL_TEXTURE_2D)
	_rectime("DF-Dist")
	
	# 3D projection mode for nearby gameplay objects with depth-testing
	cachingGlEnable(GL_DEPTH_TEST)
	cachingGlEnable(GL_LIGHTING)
	glMatrixMode(GL_PROJECTION)
	glLoadIdentity()
	gluPerspective(FOV, winsize[0]/winsize[1], 0.1, GAMEPLAY_CLIP_DIST)
	glMatrixMode(GL_MODELVIEW)
	
	# Draw all objects close enough to not be billboards
	for o in near_objs:
		o.draw()
	_rectime("DF-Objs")
	
	# 2D drawing mode
	glMatrixMode(GL_PROJECTION)
	glLoadIdentity()
	gluOrtho2D(0.0, winsize[0], winsize[1], 0.0)
	glMatrixMode(GL_MODELVIEW)
	glLoadIdentity()
	cachingGlDisable(GL_DEPTH_TEST)
	cachingGlDisable(GL_LIGHTING)
	
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

	_rectime("DF-Iface")
	
	# Output and flip buffers
	glFlush()
	pygame.display.flip()
	
	_rectime("DF-Flip")


def run():
	"""Runs the game.
	
	You have to call ui_init() and sim_init() before running this.
	
	Optionally, you may call init_area() and init_mission() to jump straight to a mission. If not, the
	game will start at the title screen.
	"""
	global totalsteps, mode, events, event_intents
	
	if cur_area is not None and cur_mission is not None:
		mode = MODE_GAMEPLAY
	else:
		mode = MODE_TITLE_SCREEN
	
	try:
		totalms = 0L #Total number of milliseconds passed in gameplay
		while True:	
			_rectime_newframe()
			elapsedms = clock.tick(maxfps)
			_rectime("Tick")
			
			events = []
			for event in pygame.event.get():
				events.append(event)
				if event.type == pygame.QUIT:
					print "Got a pygame quit event, closing app."
					raise QuitException
			
			input_man.update()
			event_intents = input_man.intents_matching_events(events)
			if inputs.INTENT_FORCE_QUIT in event_intents:
				print "Got a force quit intent, closing app."
				raise QuitException
			if inputs.INTENT_RESET_NEUTRAL in event_intents:
				print "Resetting neutral state for inputs."
				input_man.set_neutral()
			if cons.active:
				# If the console is up, don't interpret any player input as a mapped intent, except for force quit and reset neutral (handled above)
				event_intents = []
			
			_rectime("Input")
			
			if mode == MODE_GAMEPLAY and not cons.active:
				totalms += elapsedms
				#Figure out how many simulation steps we're doing this frame.
				#In theory, shouldn't ever be <1, since max frames per second is the same as steps per second
				#However, it's alright to be occasionally zero, since clock.tick is sometimes a bit off
				#FIXME: Do we really need totalms?
				steps = int(math.floor((totalms*maxfps/1000)))-totalsteps
				#Run the simulation the desired number of steps
				for i in range(steps):
					_sim_step()
					totalsteps += 1
			_rectime("Sim")
			
			# Update any active debugging watchers
			for w in watchers:
				if w.expr != None:
					w.update()
			
			#Draw everything
			_draw_frame()
	except QuitException:
		pass
