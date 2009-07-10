from __future__ import division

import ode
from OpenGL.GL import *

import app, editorexport, collision, sky, missioncon, gameobj, avatar, target
from geometry import *
from util import *

class SimpleOREGameObj(gameobj.GameObj):
	"""A GameObj which acts as a fixed piece of landscape (skyscape?), with display and collision geometry loaded from a given OREMesh.
	
	Data attributes:
	objname - The object name (i.e. "Circle.001") used in the piece of exported data this object came from. This is purely for debugging purposes.
	"""
	def __init__(self, objname, pos, rot, oremesh):
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
	player_name - A string with the player-visible name of the mission (i.e. "Following Sis")
	mission_control - An instance of missioncon.MissionControl describing mission parameters.
	objects - A sequence of mission-specific GameObjs to be appended to the GameObjs in the Area.
	"""
	def __init__(self, name, player_name, mission_control, objects):
		self.name = name
		self.player_name = player_name
		self.mission_control = mission_control
		self.objects = objects


class OREArea:
	"""Describes an area (a level) that the player can go to, loaded from an ORE data file. Each area has one or more missions.
	
	Data attributes:
	name - A string with the internal name of the area (i.e. "A01-Base")
	player_name - A string with the player-visible name of the area (i.e. "Quaternion Jungle")
	sky_stuff - The sky.SkyStuff object for this area, which is used to position it in the Smoke Ring.
	objects - A sequence of GameObjs that apply to all missions in this area; the basic geometry of the level.
	missions - A dictionary (keyed by internal name) of OREMission objects.
	"""
	def __init__(self, name, player_name, sky_stuff, objects, missions):
		self.name = name
		self.player_name = player_name
		self.sky_stuff = sky_stuff
		self.objects = objects
		self.missions = missions


class OREMesh:
	"""A mesh, with associated material, from an ORE data file. You can draw it or build an ODE TriMesh geom from it."""
	def __init__(self, eemesh, eemat):
		self._eemesh = eemesh # An editorexport.Mesh object
		self._eemat = eemat # An editorexport.Material object, or None if there is no material
		self._trimesh_data = None
		self._gl_list_num = None
	
	def trimesh_data(self):
		"""Returns an ode.TriMeshData for this mesh. This is cached after the first calculation."""
		if self._trimesh_data is None:
			self._trimesh_data = ode.TriMeshData()
			self._trimesh_data.build([v for v, n in self._eemesh.vertices], self._eemesh.faces)
		return self._trimesh_data
	
	def draw_gl(self):
		"""Draws the object using OpenGL commands. This creates a display list when it is first ran."""
		if self._gl_list_num is None:
			self._gl_list_num = glGenLists(1) # FIXME Should free list on destruction
			glNewList(self._gl_list_num, GL_COMPILE)
			if self._eemat is not None:
				glMaterialfv(GL_FRONT, GL_DIFFUSE, self._eemat.dif_col + (1.0,))
				glMaterialfv(GL_FRONT, GL_SPECULAR, self._eemat.spe_col + (1.0,))
			glBegin(GL_TRIANGLES)
			for vindexes in self._eemesh.faces:
				for vi in vindexes:
					glNormal3fv(self._eemesh.vertices[vi][1])
					glVertex3fv(self._eemesh.vertices[vi][0])
			glEnd()
			glEndList()
		
		glCallList(self._gl_list_num)


class OREManager:
	"""Manages data in the Orbit Ribbon Export format, which is just a pickle with an editorexport.ExportPackage object in it.
	
	Data attributes (read only):
	meshes - A dictionary of OREMesh instances, one for each mesh in the ORE data.
	areas - A dictionary of area.AreaDesc instances, one for each area in the ORE data.
	"""
	def __init__(self, expkg):
		self._expkg = expkg
		
		self.meshes = {}
		for meshname in self._expkg.meshes:
			mesh = self._expkg.meshes[meshname]
			matname = mesh.material
			mat = None
			if matname is not None:
				mat = self._expkg.materials[matname]
			self.meshes[meshname] = OREMesh(mesh, mat)
		
		def gobj_from_eeobj(objname, meshname, pos, rot):
			# Convert the 3-tuple of ORE rotation info (X, Y, Z floats in degrees) into a rotational matrix suitable for GameObj
			# FIXME Must implement this
			rotmatrix = (
				1, 0, 0,
				0, 1, 0,
				0, 0, 1,
			)
			ppos = Point(*pos)
			
			# FIXME Need a better way of registering special GameObjs associated with LIB objects
			if objname.startswith("LIBAvatar."):
				r = avatar.Avatar(ppos)
				# FIXME Test values
				r.rot = (0.74987399578094482, -0.65985339879989624, 0.047775015234947205, -0.18929661810398102, -0.14480534195899963, 0.97118395566940308, -0.63392096757888794, -0.73730921745300293, -0.23349379003047943)
				return r
			elif objname.startswith("LIBTargetRing."):
				return target.Ring(ppos)
			else:
				return SimpleOREGameObj(objname, ppos, rotmatrix, self.meshes[meshname])
		
		self.areas = {}
		for areaname in self._expkg.areas:
			missions = {}
			for missionname in self._expkg.areas[areaname].missions:
				ore_mission = OREMission(
					name = missionname,
					player_name = "Passing Thru The Jungle", # FIXME Test
					mission_control = missioncon.MissionControl( # FIXME Also test
						win_cond_func = missioncon.AllRingsPassedFunction(),
						timer_start_func = missioncon.MinDistanceFunction()
					),
					objects = [gobj_from_eeobj(*x) for x in self._expkg.missions[missionname].objects]
				)
				missions[missionname] = ore_mission
			
			ore_area = OREArea(
				name = areaname,
				player_name = "Quaternion Jungle", # FIXME Test
				sky_stuff = sky.SkyStuff( # FIXME Also test
					game_angle = 0.17,
					game_y_offset = 1100,
					game_d_offset = 800,
					game_tilt = (67, 0.4, 0, 0.7),
					t3_angle = 0.8,
				),
				objects = [gobj_from_eeobj(*x) for x in self._expkg.areas[areaname].objects],
				missions = missions
			)
			self.areas[areaname] = ore_area
