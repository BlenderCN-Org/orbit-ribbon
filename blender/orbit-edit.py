import Blender, BPyMesh, bpy, os, cPickle, sys, ConfigParser, StringIO, zipfile
from math import *

WORKING_DIR = os.path.dirname(Blender.Get("filename"))
BLENDER_FILENAME = os.path.basename(Blender.Get("filename"))

sys.path.append(os.path.join(WORKING_DIR, os.path.pardir))
import oreshared # This is found in the directory we just located above

MATRIX_BLEN2ORE = Blender.Mathutils.RotationMatrix(-90, 4, 'x')
MATRIX_INV_BLEN2ORE = MATRIX_BLEN2ORE.copy().invert()

def fixcoords(t): # Given a 3-sequence, returns it so that axes changed to fit OpenGL standards (y is up, z is forward)
	t = Blender.Mathutils.Vector(t) * MATRIX_BLEN2ORE
	return (t[0], t[1], t[2])


def rad2deg(v):
	return v*(180.0/pi)


def genrotmatrix(x, y, z): # Returns a 9-tuple for a column-major 3x3 rotation matrix with axes corrected ala fixcoords
	#return (1, 0, 0, 0, 1, 0, 0, 0, 1) # For testing purposes
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
	# FIXME: Need to also consider auxillary objects within LIB scenes
	# Though must keep in mind that objects in LIB scenes only have position/rotation relative to each other
	# Probably should turn Mission and Area into one class, Scene

	# Record the frame the editor was in before we started mucking about
	orig_frame = Blender.Get("curframe")
	
	target_fn = BLENDER_FILENAME[:-6] + ".ore" # Replace ".blend" with ".ore" for output filename (ore = Orbit Ribbon Export)
	zfh = zipfile.ZipFile(
		file = os.path.join(WORKING_DIR, os.path.pardir, "orefiles", target_fn),
		mode = "w",
		compression = zipfile.ZIP_DEFLATED
	)
	zfh.writestr("ore-version", "1") # Version of the ORE file format in use (this will be 1 until I make a backwards-incompatible change post release)
	
	conf = None
	try:
		conf = "\n".join(bpy.data.texts["oreconf"].asLines())
	except exceptions.KeyError:
		pup_error("Unable to find the 'oreconf' text!")
	confparser = ConfigParser.ConfigParser()
	try:
		confparser.readfp(StringIO.StringIO(conf))
	except Exception, e:
		pup_error("Unable to parse the oreconf text: %s" % str(e))
	pkgname = None
	try:
		pkgname = confparser.get("Package", "visible_name")
	except Exception, e:
		pup_error("Unable to extract 'visible_name' from '[Package]' section!'")
	zfh.writestr("ore-name", pkgname) # The player-visible name of the ORE package
	zfh.writestr("ore-conf", conf)
	
	for scene in bpy.data.scenes:
		name = scene.name
		if name.startswith("A"):
			# Either an Area base or a Mission; we need a list of (objname, meshname, position, rotmatrix) tuples
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
						fixcoords(obj.loc),
						genrotmatrix(*obj.rot),
					))
				except Exception, e:
					pup_error("Problem exporting object %s: %s" % (obj.name, str(e)))
			
			if len(exp_obj_tuples) == 0:
				continue
			if name.endswith("-Base"):
				zfh.writestr("area-%s" % name[:-5], cPickle.dumps(oreshared.Area(tuple(exp_obj_tuples)), 2))
			else:
				area_name = name[:-4] + "-Base" # Remove the "-M##" and add "-Base" to get base area scene name
				zfh.writestr("mission-%s" % name, cPickle.dumps(oreshared.Mission(area_name, tuple(exp_obj_tuples)), 2))
	
	copiedImages = set() # Set of image names that have already been copied into the zipfile
	def writeMesh(obj, tgtName):
		mesh = BPyMesh.getMeshFromObject(obj, None, True, False, None)
		vertices = [(fixcoords(v.co), fixcoords(v.no)) for v in mesh.verts]
		images = {}; nextImgIdx = 0 # The images dictionary maps image names to integers, with the first image found being 0, second 1, etc.
		uvPoints = {}; nextUvIdx = 0 # Just like the images dictionary, but for 2-tuples defining UV coordinates
		faces = []
		for f in mesh.faces:
			imgIdx, imgName = None, None
			try:
				imgName = f.image.name
			except ValueError:
				pass
			if imgName is not None:
				if imgName not in images:
					images[imgName] = nextImgIdx
					nextImgIdx += 1
					if imgName not in copiedImages:
						copiedImages.add(imgName)
						# ZIP_STORED disables compression, done here because the image already compressed
						zfh.write(f.image.filename, "image-%s" % imgName, zipfile.ZIP_STORED)
				imgIdx = images[imgName]
			
			offsets = [(0,1,2)]
			if len(f.verts) != 3:
				# Convert quads to triangles
				offsets.append((0,2,3))
			for a, b, c in offsets:
				vIdxs = (f.verts[a].index, f.verts[b].index, f.verts[c].index)
				uIdxs = (None, None, None)
				if imgIdx is not None:
					uIdxs = []
					for i in a, b, c:
						uvPt = tuple(f.uv[i])
						if uvPt not in uvPoints:
							uvPoints[uvPt] = nextUvIdx
							nextUvIdx += 1
						uIdxs.append(uvPoints[uvPt])
					uIdxs = tuple(uIdxs)
				faces.append((vIdxs, imgIdx, uIdxs))
		
		omesh = oreshared.Mesh(
			vertices = tuple(vertices),
			uvpoints = tuple([i[0] for i in sorted(uvPoints.items(), cmp = lambda a, b: cmp(a[1],b[1]))]),
			images = tuple([i[0] for i in sorted(images.items(), cmp = lambda a, b: cmp(a[1],b[1]))]),
			faces = tuple(faces),
		)
		zfh.writestr("mesh-%s" % tgtName, cPickle.dumps(omesh, 2))
	
	for obj in bpy.data.objects:
		# We're only interested in non-LIB mesh objects in missions, and also we want one copy of each LIB mesh object
		# Precondition: The LIB object in its own scene is the one that doesn't end in .###
		if obj.type == 'Mesh' and not (obj.name.startswith("LIB") and obj.name[-4] == "." and obj.name[-3:].isdigit()):
			writeMesh(obj, obj.name)
	
	for action in bpy.data.actions:
		if not action.name.startswith("LIB"):
			continue
		
		obj_name = action.name.split("-")[0]
		arm_name = obj_name + "-Armature"
		orig_action = bpy.data.objects[arm_name].getAction() # Save the selected action for the armature
		
		anim_meshes = []
		for frameNum in range(1, action.getFrameNumbers()[-1]+1):
			frame_mesh_name = "%s-%s-%04u" % (obj_name, action.name.split("-")[1], frameNum)
			anim_meshes.append(frame_mesh_name)
			Blender.Set("curframe", frameNum)
			# Precondition: LIB objects and their meshes have the same name
			writeMesh(bpy.data.objects[obj_name], frame_mesh_name)
		
		orig_action.setActive(bpy.data.objects[arm_name]) # Restore the saved action
		
		oanim = oreshared.Animation(anim_meshes)
		zfh.writestr("animation-%s" % action.name, cPickle.dumps(oanim, 2))
	
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
		menu_items[r][1]()
	Blender.Redraw()

### Execution begins here
menu()
