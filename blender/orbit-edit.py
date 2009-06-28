######################################################
#
# Demo Script for Blender 2.3 Guide
#
###################################################S68
# This script generates polygons. It is quite useless
# since you can do polygons with ADD->Mesh->Circle
# but it is a nice complete script example, and the
# polygons are 'filled'
######################################################

######################################################
# Importing modules
######################################################

import Blender
from Blender import NMesh
from Blender.BGL import *
from Blender.Draw import *

import math
from math import *

# Polygon Parameters
T_NumberOfSides = Create(3)
T_Radius = Create(1.0)

# Events
EVENT_NOEVENT = 1
EVENT_DRAW = 2
EVENT_EXIT = 3

######################################################
# GUI drawing
######################################################
def draw():
	global T_NumberOfSides
	global T_Radius
	global EVENT_NOEVENT,EVENT_DRAW,EVENT_EXIT
	
	########## Titles
	glClear(GL_COLOR_BUFFER_BIT)
	glRasterPos2d(8, 103)
	Text("Demo Polygon Script")
	
	######### Parameters GUI Buttons
	glRasterPos2d(8, 83)
	Text("Parameters:")
	T_NumberOfSides = Number("No. of sides: ", EVENT_NOEVENT, 10, 55, 210, 18, T_NumberOfSides.val, 3, 20, "Number of sides of out polygon");
	T_Radius = Slider("Radius: ", EVENT_NOEVENT, 10, 35, 210, 18, T_Radius.val, 0.001, 20.0, 1, "Radius of the polygon");
	
	######### Draw and Exit Buttons
	Button("Draw",EVENT_DRAW , 10, 10, 80, 18)
	Button("Exit",EVENT_EXIT , 140, 10, 80, 18)

def event(evt, val):	
	if (evt == QKEY and not val): 
		Exit()

def bevent(evt):
	global T_NumberOfSides
	global T_Radius
	global EVENT_NOEVENT,EVENT_DRAW,EVENT_EXIT
	
	######### Manages GUI events
	if (evt == EVENT_EXIT): 
		Exit()
	elif (evt== EVENT_DRAW):
		Polygon(T_NumberOfSides.val, T_Radius.val)
 		Blender.Redraw()

Register(draw, event, bevent)

######################################################
# Main Body
######################################################
def Polygon(NumberOfSides,Radius):

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
