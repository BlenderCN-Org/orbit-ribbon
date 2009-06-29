import Blender, bpy, os
from math import *

WORKING_DIR = os.path.dirname(Blender.Get("filename"))

def pup_error(msg):
	r = Blender.Draw.PupMenu("Error: %s%%t|OK" % msg)
	Blender.Redraw()
	Blender.Exit()

def do_add_gameobject():
	curscene = bpy.data.scenes.active
	if not curscene.name.startswith("A") or curscene.name.endswith("-Base"):
		pup_error("You must be in a Mission scene to add a GameObject Link")
	
	obj_choices = []
	for scene in bpy.data.scenes:
		name = scene.name
		if name.startswith("GO"):
			obj_choices.append(name)
	r = Blender.Draw.PupMenu("Select GameObject To Add%t|" + "|".join(["%s%%x%u" % (name, num) for num, name in enumerate(obj_choices)]))
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

def do_repop_missions():
	# First, make sure all objects in base scenes are properly named
	for scene in bpy.data.scenes:
		name = scene.name
		if name.startswith("A") and name.endswith("-Base"):
			for obj in scene.objects:
				if not obj.name.startswith("BASE"):
					obj.name = "BASE" + obj.name
	
	# Now, go through and relink base scene objects into mission scenes
	for scene in bpy.data.scenes:
		name = scene.name
		if name.startswith("A") and not name.endswith("-Base"):
			# First unlink everything from the mission scene that's from a base scene
			for obj in scene.objects:
				if obj.name.startswith("BASE"):
					scene.objects.unlink(obj)
			# Then, link all objects from the base area scene into the scene
			basename = name[:-4] + "-Base" # Remove the "-M##" and add "-Base" to get base area scene name
			basescene = bpy.data.scenes[basename]
			for obj in basescene.objects:
				scene.objects.link(obj)

def do_export():
	print "Exporting..."
	Blender.Draw.PupMenu("Exported just fine%t|OK, thanks a bunch")

def menu():
	menu = ("Add GameObject Link", "Repop Mission Scenes", "Export")
	r = Blender.Draw.PupMenu("Orbit Ribbon Level Editing%t|" + "|".join(["%s%%x%u" % (name, num) for num, name in enumerate(menu)]))
	if r >= 0:
		if menu[r] == "Add GameObject Link":
			do_add_gameobject()
		elif menu[r] == "Repop Mission Scenes":
			do_repop_missions()
		elif menu[r] == "Export":
			do_export()
	Blender.Redraw()

menu()
