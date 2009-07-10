from __future__ import division

import ode, gameobj
from OpenGL.GL import *

import area, app, editorexport, collision, sky
from util import *

def ore_rot_to_rotmatrix(tup):
	"""Converts a 3-tuple containing ORE rotation information (X, Y, Z floats in degrees) into a rotational matrix suitable for GameObj."""
	return (
		# FIXME FIXME FIXME This is where I left off working
	)


class SimpleOREGameObj(gameobj.GameObj):
	"""A GameObj which acts as a fixed piece of landscape (skyscape?) loaded from a given OREMesh.
	
	Data attributes:
	objname - The object name (i.e. "Circle.001") used in the piece of exported data this object came from. This is purely for debugging purposes.
	"""
	def __init__(self, objname, pos, rot, oremesh):
		"""Creates a SimpleOREGameObj with the given object name, position, rotation (in exportdata format), and OREMesh."""
		self.objname = objname
		self._oremesh = oremesh
		rotmatrix = ode_rot_to_rotmatrix(rot)
		geom = ode.GeomTriMesh(oremesh.trimesh_data(), app.static_space)
		geom.coll_props = collision.Props()
		super(SimpleOREGameObj, self).__init__(pos = pos, rot = rotmatrix, body = None, geom = geom)
	
	def indraw(self):
		self._oremesh.draw_gl()


class OREMesh:
	"""A mesh, with associated material, from an ORE data file. You can draw it or build an ODE TriMesh geom from it."""
	def __init__(self, eemesh, eemat):
		self._eemesh = eemesh # An editorexport.Mesh object
		self._eemat = eemat # An editorexport.Material object, or None if there is no material
		self._trimesh_data = None
	
	def trimesh_data(self):
		"""Returns an ode.TriMeshData for this mesh. This is cached after the first calculation."""
		if self._trimesh_data is None:
			self._trimesh_data = ode.TriMeshData()
			self._trimesh_data.build([v for v, n in self.vertices], self.faces)
		return self._trimesh_data
	
	def draw_gl(self):
		"""Draws the object using OpenGL commands. This is suitable for calling within display list initialization."""
		if self._eemat is not None:
			glMaterialfv(GL_FRONT, GL_DIFFUSE, self._eemat.dif_col)
			glMaterialfv(GL_FRONT, GL_SPECULAR, self._eemat.spe_col)
		glBegin(GL_TRIANGLES)
		for vindexes in self._eemesh._faces:
			for vi in vindexes:
				glNormal3fv(self._eemesh.vertices[i][1])
				glVertex3fv(self._eemesh.vertices[i][0])
		glEnd()


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
		
		# FIXME Areas need to contain missions, which implies I should probably add a MissionDesc class
		# That in turn means I probably need to rename the 'area' module to something more general
		# Maybe I should just merge it with the ore module
		
		self.areas = []
		for areaname in self._expkg.areas:
			areadesc = area.AreaDesc(
				name = areaname,
				player_name = "Quaternion Jungle", # FIXME Test
				sky_stuff = sky.SkyStuff( # FIXME Also test
					game_angle = 0.17,
					game_y_offset = 1100,
					game_d_offset = 800,
					game_tilt = (67, 0.4, 0, 0.7),
					t3_angle = 0.8,
				),
				# Don't need to worry about special LIB objects in Area scenes, as of the current spec anyways
				objects = [ SimpleOREGameObj(objname, pos, rot, self.meshes[meshname]) for objname, meshname, pos, rot in self._expkg.areas[areaname].objects ]
			)
			self.areas[areaname] = areadesc
