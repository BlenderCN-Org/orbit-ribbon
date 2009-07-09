import Blender, bpy, os, cPickle, sys
from math import *

WORKING_DIR = os.path.dirname(Blender.Get("filename"))
BLENDER_FILE = os.path.basename(Blender.Get("filename"))

sys.path.append(os.path.join(WORKING_DIR, os.path.pardir))

import editorexport

def pup_error(msg):
	r = Blender.Draw.PupMenu("Error: %s%%t|OK" % msg)
	Blender.Redraw()
	Blender.Exit()

def do_add_libobject():
	curscene = bpy.data.scenes.active
	if not curscene.name.startswith("A") or curscene.name.endswith("-Base"):
		pup_error("You must be in a Mission scene to add a Library Object")
	
	obj_choices = []
	for scene in bpy.data.scenes:
		name = scene.name
		if name.startswith("LIB"):
			obj_choices.append(name)
	r = Blender.Draw.PupMenu("Select Library Object To Add%t|" + "|".join(["%s%%x%u" % (name, num) for num, name in enumerate(obj_choices)]))
	if r != -1:
		choice = obj_choices[r]
		src_obj = None
		for obj in bpy.data.scenes[choice].objects:
			if obj.name == choice:
				src_obj = obj
				break
		if src_obj is None:
			pup_error("Unable to find object named %s in scene %s" % (choice, choice))
		new_obj = curscene.objects.new(src_obj.data)
		new_obj.size = src_obj.size
		new_obj.loc = curscene.cursor
		for obj in curscene.objects:
			obj.sel = False
		new_obj.sel = True

def do_resanify():
	# First, make sure all objects in base scenes are properly named
	for scene in bpy.data.scenes:
		name = scene.name
		if name.endswith("-Base"):
			for obj in scene.objects:
				if not obj.name.startswith("BASE"):
					obj.name = "BASE" + obj.name
	
	# Relink lights into all LIB scenes and all other base scenes
	lightingbase = bpy.data.scenes["TestLighting-Base"]
	for scene in bpy.data.scenes:
		name = scene.name
		if not (name.startswith("LIB") or name.endswith("Base")):
			continue
		if name == "TestLighting-Base":
			continue
		# Unlink all the lights from this scene
		to_be_unlinked = []
		for obj in scene.objects:
			if obj.name.startswith("BASETestLamp"):
				to_be_unlinked.append(obj)
		for obj in to_be_unlinked:
			scene.objects.unlink(obj)
		for obj in lightingbase.objects:
			scene.objects.link(obj)
	
	# Relink all base scene objects into mission scenes (this includes the lights linked into base scenes by the previous step)
	for scene in bpy.data.scenes:
		name = scene.name
		if name.endswith("-Base"):
			continue
		elif name.startswith("A"):
			# This is a mission scene, link all objects from the base area scene into the scene
			basename = name[:-4] + "-Base" # Remove the "-M##" and add "-Base" to get base area scene name
			basescene = bpy.data.scenes[basename]
			# First unlink everything in the target scene that's from a base scene; some of these might've been deleted
			to_be_unlinked = []
			for obj in scene.objects:
				if obj.name.startswith("BASE"):
					to_be_unlinked.append(obj)
			for obj in to_be_unlinked:
				scene.objects.unlink(obj)
			# Now link all the base objects in
			for obj in basescene.objects:
				scene.objects.link(obj)
	
	def unscale_obj_mesh(o):
		mesh = bpy.data.meshes[o.getData().name]
		for vertex in mesh.verts:
			vertex.co[:] = [vertex.co[i]*o.size[i] for i in range(3)]
	
	# In all scenes, remove all scale from objects
	# This way we don't have to worry about ODE being unable to rescale geoms, among other complications
	for obj in bpy.data.objects:
		if obj.type != "Mesh":
			# Don't care about this object, since it cannot be rescaled
			continue
		if obj.size == (1.0, 1.0, 1.0):
			# This object already has no scale
			continue
		if obj.name.startswith("LIB"):
			# If it's an original LIB object, unscale its mesh and unscale the object
			# If it's a linked LIB object in a mission scene, then just remove scale from the object
			if not (obj.name[-4] == "." and obj.name[-3:].isdigit()): # If it doesn't end in ".###":
				unscale_obj_mesh(obj)
			obj.size = (1.0, 1.0, 1.0)
		else:
			# Here we have a choice; resize the mesh or not
			# Ask the user what to do
			r = Blender.Draw.PupMenu(
				"Need to unscale OB:%s with ME:%s. Do what?%%t|Unscale object and adjust mesh%%x0|Unscale object only%%x1|Ignore the problem%%x2" % 
				(obj.name, obj.getData().name)
			)
			if r == 0 or r == 1:
				if r == 0:
					unscale_obj_mesh(obj)
				obj.size = (1.0, 1.0, 1.0)
	
	Blender.Draw.PupMenu("Resanification went OK%t|Yeah man, cool")

def do_export():
	meshes, materials, areas, missions = {}, {}, {}, {}
	
	# FIXME: Need to also consider auxillary objects within LIB scenes
	# Though must keep in mind that objects in LIB scenes only have position/rotation relative to each other
	# Probably should turn Mission and Area into one class, Scene
	
	for scene in bpy.data.scenes:
		name = scene.name
		if name.startswith("A"):
			# Either an Area base or a Mission; we need a list of (objname, meshname, position, rotation) tuples
			exp_obj_tuples = []
			filt = None
			
			if name.endswith("-Base"):
				# Area base scene: log all non-TestLamp BASE objects
				filt = lambda n: True if (n.startswith("BASE") and not ("TestLamp") in n) else False
			else:
				# Mission scene: log all non-BASE objects (whether LIB or not)
				filt = lambda n: True if not n.startswith("BASE") else False
			
			for obj in scene.objects:
				if not filt(obj.name):
					continue
				try:
					exp_obj_tuples.append((
						obj.name,
						obj.getData().name,
						(obj.loc[0], obj.loc[1], obj.loc[2]),
						(obj.rot[0], obj.rot[1], obj.rot[2]),
					))
				except Exception, e:
					pup_error("Problem exporting object %s: %s" % (obj.name, str(e)))
			
			if len(exp_obj_tuples) == 0:
				continue
			if name.endswith("-Base"):
				areas[name] = editorexport.Area(tuple(exp_obj_tuples))
			else:
				area_name = name[:-4] + "-Base" # Remove the "-M##" and add "-Base" to get base area scene name
				missions[name] = editorexport.Mission(area_name, tuple(exp_obj_tuples))
	
	for mesh in bpy.data.meshes:
		name = mesh.name
		matName = None
		if len(mesh.materials) > 0:
			matName = mesh.materials[0].name
		
		meshes[name] = editorexport.Mesh(
			vertices = tuple([(tuple(v.co), tuple(v.no)) for v in mesh.verts]),
			faces = tuple([tuple([v.index for v in f.verts]) for f in mesh.faces]),
			material = matName
		)
		
		if matName is not None and matName not in materials:
			mat = bpy.data.materials[matName]
			materials[matName] = editorexport.Material(
				dif_col = tuple(mat.rgbCol),
				spe_col = tuple(mat.specCol),
			)
	
	pkg = editorexport.ExportPackage(meshes, materials, areas, missions)
	target_fn = BLENDER_FILE[:-6] + ".ore" # Replace ".blend" with ".ore" for output filename (ore = Orbit Ribbon Export)
	fh = file(os.path.join(WORKING_DIR, os.path.pardir, "exportdata", target_fn), "wb")
	cPickle.dump(pkg, fh, 2)
	fh.close()
	
	Blender.Draw.PupMenu("Exported just fine%t|OK, thanks a bunch")

def menu():
	menu = ("Add Library Object", "Resanify All Scenes", "Export")
	r = Blender.Draw.PupMenu("Orbit Ribbon Level Editing%t|" + "|".join(["%s%%x%u" % (name, num) for num, name in enumerate(menu)]))
	if r >= 0:
		if menu[r] == "Add Library Object":
			do_add_libobject()
		elif menu[r] == "Resanify All Scenes":
			do_resanify()
		elif menu[r] == "Export":
			do_export()
	Blender.Redraw()

### Execution begins here
menu()
