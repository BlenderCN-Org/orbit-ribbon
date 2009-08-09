from __future__ import division

import ode, ConfigParser, StringIO, pickle, zipfile, cPickle
import OpenGL
OpenGL.ERROR_CHECKING = False
OpenGL.ERROR_LOGGING = False
from OpenGL.GL import *

import app, oreshared, collision, sky, missioncon, gameobj, avatar, target, resman
from geometry import *
from util import *

class SimpleOREGameObj(gameobj.GameObj):
	"""A GameObj which acts as a fixed piece of landscape (skyscape?), with display and collision geometry loaded from a given OREMesh.
	
	Data attributes:
	objname - The object name (i.e. "Circle.001") used in the piece of exported data this object came from. This is kept purely for debugging purposes.
	"""
	def __init__(self, oremesh, objname, pos, rot):
		"""Creates a SimpleOREGameObj with the given object name, position, rotation (in exportdata format), and OREMesh."""
		self.objname = objname
		self._oremesh = oremesh
		geom = ode.GeomTriMesh(oremesh.trimesh_data(), app.static_space)
		geom.coll_props = collision.Props()
		super(SimpleOREGameObj, self).__init__(pos = pos, rot = rot, body = None, geom = geom)
	
	def indraw(self):
		self._oremesh.draw_gl()


class OREMission:
	"""Describes a mission that the player can attempt, loaded from an ORE data file. Each mission takes place in a particular area.
	
	Data attributes:
	name - A string with the internal name of the mission (i.e. "A01-M02")
	visible_name - A string with the player-visible name of the mission (i.e. "Following Sis")
	mission_control - An instance of missioncon.MissionControl describing mission parameters.
	objects - A sequence of mission-specific GameObjs to be appended to the GameObjs in the Area.
	"""
	def __init__(self, name, visible_name, mission_control, objects):
		self.name = name
		self.visible_name = visible_name
		self.mission_control = mission_control
		self.objects = objects


class OREArea:
	"""Describes an area (a level) that the player can go to, loaded from an ORE data file. Each area has one or more missions.
	
	Data attributes:
	name - A string with the internal name of the area (i.e. "A01")
	visible_name - A string with the player-visible name of the area (i.e. "Quaternion Jungle")
	sky_stuff - The sky.SkyStuff object for this area, which is used to position it in the Smoke Ring.
	objects - A sequence of GameObjs that apply to all missions in this area; the basic geometry of the level.
	missions - A dictionary (keyed by internal name) of OREMission objects.
	"""
	def __init__(self, name, visible_name, sky_stuff, objects, missions):
		self.name = name
		self.visible_name = visible_name
		self.sky_stuff = sky_stuff
		self.objects = objects
		self.missions = missions


class OREMesh:
	"""A mesh, with associated material, from an ORE data file. You can draw it or build an ODE TriMesh geom from it."""
	def __init__(self, oreshr_mesh, zfh):
		self._oreshr_mesh = oreshr_mesh # An oreshared.Mesh object
		self._zfh = zfh # A zipfile handle from which we can load images
		self._trimesh_data = None
		self._texdict = {}
		
		self._gl_list_num = glGenLists(1) # TODO Should free list on destruction
		glNewList(self._gl_list_num, GL_COMPILE)
		self._draw_gl_impl()
		glEndList()
	
	def trimesh_data(self):
		"""Returns an ode.TriMeshData for this mesh. This is cached after the first calculation."""
		if self._trimesh_data is None:
			self._trimesh_data = ode.TriMeshData()
			self._trimesh_data.build([v for v, n in self._oreshr_mesh.vertices], [f for f, i, u in self._oreshr_mesh.faces])
		return self._trimesh_data
	
	def draw_gl(self):
		"""Draws the object using OpenGL commands."""
		glCallList(self._gl_list_num)
	
	def _draw_gl_impl(self):
		curImg = None
		# Pick a noticeable purple color for untextured meshes
		glMaterialfv(GL_FRONT, GL_DIFFUSE, (1.0, 0.0, 1.0, 1.0,))
		glMaterialfv(GL_FRONT, GL_SPECULAR, (0.1, 0.0, 0.1, 1.0,))
		glBegin(GL_TRIANGLES)
		for vertIdxs, imgIdx, uvIdxs in self._oreshr_mesh.faces:
			if curImg != imgIdx:
				# Got to change textures, leave vertex specification mode for a moment
				glEnd()
				if imgIdx is None:
					# No image assigned to this face? Pick a noticeable purple color instead
					glDisable(GL_TEXTURE_2D)
					glMaterialfv(GL_FRONT, GL_DIFFUSE, (1.0, 0.0, 1.0, 1.0,))
					glMaterialfv(GL_FRONT, GL_SPECULAR, (0.1, 0.0, 0.1, 1.0,))
				else:
					if curImg is None:
						# If textures aren't already enabled, enable them
						glEnable(GL_TEXTURE_2D)
					imgName = self._oreshr_mesh.images[imgIdx]
					# TODO This is very messy. Would be better to refactor resman together with ore
					glBindTexture(GL_TEXTURE_2D, resman.Texture("ORE-%s" % imgName, lambda: self._zfh.read("image-%s" % imgName)).glname)
				curImg = imgIdx
				glBegin(GL_TRIANGLES) # Okay, back to specifying vertices
			for vi, ui in zip(vertIdxs, uvIdxs):
				if ui is not None:
					glTexCoord2fv(self._oreshr_mesh.uvpoints[ui])
				glNormal3fv(self._oreshr_mesh.vertices[vi][1])
				glVertex3fv(self._oreshr_mesh.vertices[vi][0])
		glEnd()
		glDisable(GL_TEXTURE_2D)


class OREAnimation:
	"""An animation loaded from the ORE file, consisting of the OREMeshes that make up its frames.
	
	Data attributes:
	frames - A sequence of OREMesh instances, one for each frame of the animation.
	"""
	def __init__(self, oreshr_anim, zfh):
		self._oreshr_anim = oreshr_anim # An oreshared.Animation object
		self._zfh = zfh # A zipfile handle from which we can load meshes
		self.frames = []
		for frame_name in self._oreshr_anim.frames:
			mesh = cPickle.loads(zfh.read("animmesh-%s" % frame_name))
			self.frames.append(OREMesh(mesh, zfh))


# TODO Should have the ability to close ORE files on demand, so that we can switch to another ORE file without closing the game
class OREManager:
	"""Loads and interprets data in the Orbit Ribbon Export format.
	
	This format is actually just a zipfile containing various files describing areas, missions, object meshes, texture images, etc.
	
	Data attributes (read only):
	visible_name - The player-appropriate name of the ORE file.
	meshes - A dictionary of OREMesh instances, one for each mesh in the ORE data.
	animations - A dictionary of OREAnimation instances, one for each animation in the ORE data.
	areas - A dictionary of OREArea instances, one for each area in the ORE data.
	"""
	def __init__(self, fh):
		"""Creates an OREManager based on the data in the given filehandle."""
		zfh = zipfile.ZipFile(fh)
		fnames = {} # Lists of names ordered by section, so that i.e. fnames["mesh"]["donut"] exists for a file named "mesh-donut"
		for zfn in zfh.namelist():
			dashidx = zfn.find("-")
			if dashidx == -1:
				raise RuntimeError()
			secname, subname = zfn[:dashidx], zfn[(dashidx+1):]
			if secname not in fnames:
				fnames[secname] = []
			fnames[secname].append(subname)
		
		# Make sure we got an ORE with a version number we understand
		vnum = int(zfh.read("ore-version"))
		if vnum != 1:
			raise RuntimeError("Unknown ORE file version '%u'. Install a newer version of the game to use this file." % vnum)
		
		# Load the config from the ExportPackage
		cparser = ConfigParser.ConfigParser()
		cparser.readfp(StringIO.StringIO(zfh.read("ore-conf")))

		self.meshes = {}
		for meshname in fnames["mesh"]:
			# FIXME Temporary speedup
			if "JungleA.001" in meshname:
				continue
			mesh = cPickle.loads(zfh.read("mesh-%s" % meshname))
			self.meshes[meshname] = OREMesh(mesh, zfh)
		
		self.animations = {}
		for animname in fnames["animation"]:
			# FIXME Temporary speedup
			if ("-Run") in animname or "-PrerunToRun" in animname:
				continue
			anim = cPickle.loads(zfh.read("animation-%s" % animname))
			self.animations[animname] = OREAnimation(anim, zfh)
		
		def gobj_from_oreobj(objname, meshname, pos, rot):
			ppos = Point(*pos)
			
			# FIXME Need a better way of registering special GameObjs associated with LIB objects
			if objname.startswith("LIBAvatar."):
				return avatar.Avatar(self, ppos, rot)
			elif objname.startswith("LIBTargetRing."):
				return target.Ring(self, ppos, rot)
			else:
				return SimpleOREGameObj(self.meshes[meshname], objname, ppos, rot)
		
		self.areas = {}
		for areaname in fnames["area"]:
			area = cPickle.loads(zfh.read("area-%s" % areaname))
			
			missions = {}
			for missionname in fnames["mission"]:
				if not missionname.startswith("%s-" % areaname):
					continue
				mission = cPickle.loads(zfh.read("mission-%s" % missionname))
				ore_mission = OREMission(
					name = missionname,
					visible_name = cparser.get(missionname, "visible_name"),
					mission_control = missioncon.MissionControl(
						win_cond_func = missioncon.__dict__[cparser.get(missionname, "win_cond_func")](),
						timer_start_func = missioncon.__dict__[cparser.get(missionname, "timer_start_func")](),
					),
					objects = [gobj_from_oreobj(*x) for x in mission.objects]
				)
				missions[missionname] = ore_mission
			
			ore_area = OREArea(
				name = areaname,
				visible_name = cparser.get(areaname, "visible_name"),
				sky_stuff = sky.SkyStuff(
					game_angle = cparser.getfloat(areaname, "sky_game_angle"),
					game_y_offset = cparser.getfloat(areaname, "sky_game_y_offset"),
					game_d_offset = cparser.getfloat(areaname, "sky_game_d_offset"),
					game_tilt = (
						cparser.getfloat(areaname, "sky_game_tilt_deg"),
						cparser.getfloat(areaname, "sky_game_tilt_x"),
						0,
						cparser.getfloat(areaname, "sky_game_tilt_z"),
					),
					t3_angle = 0.8,
				),
				objects = [gobj_from_oreobj(*x) for x in area.objects],
				missions = missions
			)
			self.areas[areaname] = ore_area
