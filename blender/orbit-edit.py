# -*- coding: utf-8 -*-

"""
orbit-edit.py: Blender utility script for doing Orbit Ribbon development

Copyright 2009 David Simon. You can reach me at david.mike.simon@gmail.com

This file is part of Orbit Ribbon.

Orbit Ribbon is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Orbit Ribbon is distributed in the hope that it will be awesome,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Orbit Ribbon.  If not, see http://www.gnu.org/licenses/
"""

import Blender, BPyMesh, bpy, os, sys, zipfile, datetime, lxml.etree, traceback
from math import *

WORKING_DIR = os.path.dirname(Blender.Get("filename"))
BLENDER_FILENAME = os.path.basename(Blender.Get("filename"))

MATRIX_BLEN2ORE = Blender.Mathutils.RotationMatrix(-90, 4, 'x')
MATRIX_INV_BLEN2ORE = MATRIX_BLEN2ORE.copy().invert()

ORE_NAMESPACE = "http://www.orbit-ribbon.org/ORE1"
ORE_NS_PREFIX = "{%s}" % ORE_NAMESPACE
NSMAP = {"ore" : ORE_NAMESPACE}

def fixcoords(t): # Given a 3-sequence, returns it so that axes changed to fit OpenGL standards (y is up, z is forward)
	t = Blender.Mathutils.Vector(t) * MATRIX_BLEN2ORE
	return (t[0], t[1], t[2])


def rad2deg(v):
	return v*(180.0/pi)


def genrotmatrix(x, y, z): # Returns a 9-tuple for a column-major 3x3 rotation matrix with axes corrected ala fixcoords
	m = (
		MATRIX_INV_BLEN2ORE *
		Blender.Mathutils.RotationMatrix(rad2deg(x), 4, 'x') *
		Blender.Mathutils.RotationMatrix(rad2deg(y), 4, 'y') *
		Blender.Mathutils.RotationMatrix(rad2deg(z), 4, 'z') *
		MATRIX_BLEN2ORE
	)
	return ( # Elide final column and row
		m[0][0], m[0][1], m[0][2],
		m[1][0], m[1][1], m[1][2],
		m[2][0], m[2][1], m[2][2],
	)


def pup_error(msg):
	Blender.Window.DrawProgressBar(1.0, "Aborting")
	r = Blender.Draw.PupMenu("Error: %s%%t|OK" % msg)
	Blender.Redraw()
	raise RuntimeError(msg)


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
	# FIXME: Need to also consider auxillary objects within LIB scenes
	# Though must keep in mind that objects in LIB scenes only have position/rotation relative to each other
	
	# Load the schemas we need
	def schema_load(sname):
		return lxml.etree.XMLSchema(lxml.etree.parse(os.path.join(WORKING_DIR, os.path.pardir, "xml", sname)))
	descSchema = schema_load("orepkgdesc.xsd")
	#meshSchema = schema_load("oremesh.xsd")
	#animationSchema = schema_load("oreanimation.xsd")
	
	Blender.Window.DrawProgressBar(0.0, "Initializing export")
	
	# Load the description from TX:oredesc, and validate it against the description schema
	descDoc = lxml.etree.fromstring(
		"\n".join(bpy.data.texts["oredesc"].asLines()),
		parser=lxml.etree.XMLParser(remove_blank_text=True, schema=descSchema),
	)
	
	# Record the frame the editor was in before we started mucking about
	orig_frame = Blender.Get("curframe")
	
	Blender.Window.DrawProgressBar(0.1, "Creating ORE file")
	
	# Create a new ORE zip file
	target_fn = BLENDER_FILENAME[:-6] + ".ore" # Replace ".blend" with ".ore" for output filename (ore = Orbit Ribbon Export)
	zfh = zipfile.ZipFile(
		file = os.path.join(WORKING_DIR, os.path.pardir, "orefiles", target_fn),
		mode = "w",
		compression = zipfile.ZIP_DEFLATED
	)
	zfh.writestr("ore-version", "1") # ORE file format version (this will be 1 until the first backwards-incompatible change after release)
	zfh.writestr("ore-name", descDoc.xpath("niceName/text()")[0]) # The nice name of the ORE package, separate so that we can get it without XML
	
	# Add the mission and area information to the desc document
	for scene in bpy.data.scenes:
		name = scene.name
		if name.startswith("A"):
			filt = None # Expression used to determine if a Blender object should be exported
			tgt = None # An XML node to insert the object nodes underneath
			
			if name.endswith("-Base"):
				# Area base scene: write all non-TestLamp BASE objects to the Area node
				filt = lambda n: True if (n.startswith("BASE") and not ("TestLamp") in n) else False
				tgt = descDoc.xpath("area[@name='%s']" % (name[:3]))[0] # Use the 'A##' name
			else:
				# Mission scene: write all non-BASE objects (whether LIB or not) to the Mission node under the Area node
				filt = lambda n: True if not n.startswith("BASE") else False
				tgt = descDoc.xpath("area[@name='%s']/mission[@name='%s']" % (name[:3], name[4:]))[0] # Use the 'A##' and 'M##' names
			
			for obj in scene.objects:
				if not filt(obj.name):
					continue
				try:
					objNode = lxml.etree.SubElement(tgt, "obj", objName=obj.name, meshName=obj.getData().name)
					posNode = lxml.etree.SubElement(objNode, "pos"); posNode.text = " ".join([str(x) for x in fixcoords(obj.loc)])
					rotNode = lxml.etree.SubElement(objNode, "rot"); rotNode.text = " ".join([str(x) for x in genrotmatrix(*obj.rot)])
				except Exception, e:
					pup_error("Problem exporting object %s: %s" % (obj.name, str(e)))
	
	# Write out the description now that all the area and mission information has been added in
	#descSchema.assertValid(descDoc)
	zfh.writestr("ore-desc", lxml.etree.tostring(descDoc, xml_declaration=True))
	
	copiedImages = set() # Set of image names that have already been copied into the zipfile
	def populateMeshNode(meshNode, mesh):
		for f in mesh.faces:
			imgName = None
			try:
				imgName = f.image.name
			except ValueError:
				imgName = "" # No image mapped to this face
			
			facelistResult = meshNode.xpath("facelist[@image='%s']" % imgName)
			facelistNode = None
			if len(facelistResult) > 0:
				facelistNode = facelistResult[0]
			else:
				facelistNode = lxml.etree.SubElement(meshNode, "facelist", image=imgName)
				if imgName is not "" and imgName not in copiedImages:
					copiedImages.add(imgName)
					# ZIP_STORED disables compression, it is used here because PNG images are already compressed
					zfh.write(f.image.filename, "image-%s" % imgName, zipfile.ZIP_STORED)
			
			offsets = [(0,1,2)]
			if len(f.verts) != 3:
				# Convert quads to triangles
				offsets.append((0,2,3))
			for a, b, c in offsets:
				faceNode = lxml.etree.SubElement(facelistNode, "face")
				for offset in (a, b, c):
					vxNode = lxml.etree.SubElement(faceNode, "v")
					ptNode = lxml.etree.SubElement(vxNode, "p"); ptNode.text = " ".join([str(x) for x in fixcoords(f.verts[offset].co)])
					nmNode = lxml.etree.SubElement(vxNode, "n"); nmNode.text = " ".join([str(x) for x in fixcoords(f.verts[offset].no)])
					if imgName != "":
						txNode = lxml.etree.SubElement(vxNode, "t"); txNode.text = " ".join([str(x) for x in f.uv[offset]])
		
		return meshNode
	
	progress = 0.2
	progressInc = (1.0 - progress)/(len(bpy.data.objects) + len(bpy.data.actions))
	
	for mesh in bpy.data.meshes:
		Blender.Window.DrawProgressBar(progress, "Exporting object meshes and images")
		progress += progressInc
		meshNode = lxml.etree.Element(ORE_NS_PREFIX + "mesh", nsmap=NSMAP)
		populateMeshNode(meshNode, mesh)
		#meshSchema.assertValid(meshNode)
		zfh.writestr("mesh-%s" % mesh.name, lxml.etree.tostring(meshNode, xml_declaration=True))
	
	for action in bpy.data.actions:
		if not action.name.startswith("LIB"):
			continue
		
		obj_name = action.name.split("-")[0]
		arm_name = obj_name + "-Armature"
		orig_action = bpy.data.objects[arm_name].getAction() # Save the selected action for the armature
		
		animNode = lxml.etree.Element(ORE_NS_PREFIX + "animation", name=action.name, nsmap=NSMAP)
		
		frameCount = action.getFrameNumbers()[-1]
		subProgressInc = progressInc/frameCount
		for frameNum in xrange(1, frameCount+1):
			Blender.Window.DrawProgressBar(progress, "Exporting animation meshes")
			progress += subProgressInc
			Blender.Set("curframe", frameNum)
			mesh = BPyMesh.getMeshFromObject(bpy.data.objects[obj_name], None, True, False, None)
			meshNode = lxml.etree.SubElement(animNode, "frame")
			populateMeshNode(meshNode, mesh)
		
		orig_action.setActive(bpy.data.objects[arm_name]) # Restore the saved action
		
		#animationSchema.assertValid(animNode)
		zfh.writestr("animation-%s" % action.name, lxml.etree.tostring(animNode, xml_declaration=True))
	
	Blender.Window.DrawProgressBar(1.0, "Closing ORE file")
	zfh.close()
	Blender.Set("curframe", orig_frame) # Restore saved frame
	Blender.Draw.PupMenu("Exported just fine%t|OK, thanks a bunch")


def do_run_game():
	curscene = bpy.data.scenes.active.name
	args = []
	if curscene.startswith("A") and not curscene.endswith("-Base"):
		# We're on a mission scene, so we can have the game jump right to it
		r = Blender.Draw.PupMenu("Start the game where?%%t|At mission %s%%x0|At the title screen%%x1" % curscene)
		if r < 0:
			return
		elif r == 0:
			args.append("-m")
			args.append(curscene)
	os.system(" ".join((os.path.join(WORKING_DIR, os.pardir, "orbit-ribbon.py"),) + tuple(args)))


def menu():
	menu_items = (
		("Add Library Object", do_add_libobject),
		("Resanify All Scenes", do_resanify),
		("Export", do_export),
		("Run Game", do_run_game),
	)
	r = Blender.Draw.PupMenu("Orbit Ribbon Level Editing%t|" + "|".join(["%s%%x%u" % (name, num) for num, (name, func) in enumerate(menu_items)]))
	if r >= 0:
		try:
			menu_items[r][1]()
		except:
			print "-----"
			print "EXCEPTION TRACEBACK:"
			traceback.print_exc()
			print "-----"
			pup_error("DANGER WILL ROBINSON! OPERATION FAILED!")
	Blender.Redraw()

### Execution begins here
menu()
