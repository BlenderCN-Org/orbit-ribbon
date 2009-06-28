import Blender
from Blender import NMesh
from Blender.BGL import *
from Blender.Draw import *

import os
from math import *

WORKING_DIR = os.path.dirname(Blender.Get("filename"))

# Events
EVENT_NOEVENT, EVENT_ADD_GO, EVENT_EXPORT, EVENT_QUIT = range(4)

def list_gameobj_blends():
	"""Returns a list of blend files which (appear to) contain game objects."""
	ret = []
	for fn in os.listdir(WORKING_DIR):
		if fn.startswith("go-") and fn.endswith(".blend"):
			ret.append(fn)
	return ret


def polygon(NumberOfSides,Radius):
	######### Creates a new mesh
	poly = NMesh.GetRaw()
	
	######### Populates it of vertices
	for i in range(0,NumberOfSides):
		phi = 3.141592653589 * 2 * i / NumberOfSides
		x = Radius * cos(phi)
		y = Radius * sin(phi)
		z = 0
		
		v = NMesh.Vert(x,y,z)
		poly.verts.append(v)
	
	######### Adds a new vertex to the center
	v = NMesh.Vert(0.,0.,0.)
	poly.verts.append(v)
	
	######### Connects the vertices to form faces
	for i in range(0,NumberOfSides):
		f = NMesh.Face()
		f.v.append(poly.verts[i])
		f.v.append(poly.verts[(i+1)%NumberOfSides])
		f.v.append(poly.verts[NumberOfSides])
		poly.faces.append(f)
	
	######### Creates a new Object with the new Mesh
	polyObj = NMesh.PutRaw(poly)
	
	Blender.Redraw()


def gui_draw():
	X_MARGIN = 10
	Y_STEP = 23
	HEIGHT = 18
	WIDTH = 150
	
	glClear(GL_COLOR_BUFFER_BIT)
	
	# Due to OpenGL conventions, we draw from the bottom to the top
	y = Y_STEP - HEIGHT
	Button("Export", EVENT_EXPORT, X_MARGIN, y, WIDTH, HEIGHT)
	y += Y_STEP
	Button("Add GameObject", EVENT_ADD_GO, X_MARGIN, y, WIDTH, HEIGHT)
	y += Y_STEP
	Label("Orbit Ribbon Level Editor", X_MARGIN, y, WIDTH, HEIGHT)


def gui_event(evt, val):	
	if evt == QKEY and not val: 
		Exit()


def gui_bevent(evt):
	if evt == EVENT_EXPORT:
		do_export()
	elif evt == EVENT_ADD_GO:
		gameobj_blends = list_gameobj_blends()
		r = Blender.Draw.PupMenu("Select GameObject%t|" + "|".join(["%s%%x%u" % (name, num) for num, name in enumerate(gameobj_blends)]))
		if r >= 0:
			do_add_gameobj(gameobj_blends[r])
	if evt != EVENT_NOEVENT:
		Blender.Redraw()


Register(gui_draw, gui_event, gui_bevent)

def do_export():
	print "Exporting..."

def do_add_gameobj(blend_fn):
	print "Adding Game Object from %s" % blend_fn
