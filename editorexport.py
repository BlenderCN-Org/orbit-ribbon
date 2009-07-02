from __future__ import division

import ode

from geometry import *

class Mesh:
	"""A mesh (collection of vertices and faces w/ normals) exported from the 3D editor.
	
	Data attributes:
	vertices - A list of vertices as Points.
	normals - A list of vertex normal vectors as Points.
	faces - A list of faces as ((vi, vi, vi), (n, n, n)), where vi is an index into vertices, and ni is an index into normals.
	material - A string describing the name of the Material for this Mesh, or None if there is no Material.
	"""
	def __init__(self, vertices, normals, faces, material):
		self.vertices = vertices
		self.normals = normals
		self.faces = faces
		self.material = material
		self._trimesh_data = None
	
	def trimesh_data(self):
		"""Returns an ode.TriMeshData for this mesh. This is cached after the first calculation."""
		if self._trimesh_data is None:
			self._trimesh_data = ode.TriMeshData()
			tdat.build(self.vertices, [f[1] for f in self.faces])
		return self._trimesh_data


class Material:
	"""A material (texture and appearance) exported from the 3D editor.
	
	Data attributes:
	amb_col, dif_col, spe_col - The ambient, diffuse, and specular colors, each as 4-tuples.
	"""
	def __init__(self, amb_col, dif_col, spe_col):
		self.amb_col = amb_col
		self.dif_col = dif_col
		self.spe_col = spe_col


class Area:
	"""A description of an in-game location, exported from the 3D editor.
	
	Data attributes:
	objects - A list of area objects as (meshname, position, rotation).
	"""
	def __init__(self, objects):
		self.objects = objects
