import Blender, os
from math import *

WORKING_DIR = os.path.dirname(Blender.Get("filename"))

def list_gameobj_blends():
	"""Returns a list of blend files which (appear to) contain game objects."""
	ret = []
	for fn in os.listdir(WORKING_DIR):
		if fn.startswith("go-") and fn.endswith(".blend"):
			ret.append(fn)
	return ret


def do_export():
	print "Exporting..."
	Blender.Draw.PupMenu("Exported Just Fine%t|OK")


def do_add_gameobj(blend_fn):
	print "Adding Game Object from %s" % blend_fn
	short_name = blend_fn[3:-6] # Extract the "go-" and the ".blend" from the filename
	
	# Find the lowest-numbered object we can get
	obj_name_list = [x.name for x in Blender.Object.Get()]
	n = 1
	while 1:
		obj_name = "GO-%s%04u" % (short_name, n)
		if obj_name not in obj_name_list:
			break
	obj = Blender.Object.New("Mesh") # Adding a new Mesh Object (as distinct from a new Mesh)
	
	Blender.Library.Open(os.path.join(WORKING_DIR, blend_fn))
	mesh = Blender.Library.Load("GameObj", "Mesh")
	Blender.Library.Close()


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


def menu():
	menu = ("Add GameObject", "Export")
	r = Blender.Draw.PupMenu("Orbit Ribbon Level Editing%t|" + "|".join(["%s%%x%u" % (name, num) for num, name in enumerate(menu)]))
	if r >= 0:
		if menu[r] == "Add GameObject":
			gameobj_blends = list_gameobj_blends()
			sub_r = Blender.Draw.PupMenu("Select GameObject%t|" + "|".join(["%s%%x%u" % (name, num) for num, name in enumerate(gameobj_blends)]))
			if sub_r >= 0:
				do_add_gameobj(gameobj_blends[sub_r])
		elif menu[r] == "Export":
			do_export()
	Blender.Redraw()

menu()
