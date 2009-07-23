from __future__ import division

### This is the common library connecting orbit-edit.py (which depends on the Blender API) and the rest of Orbit Ribbon via ore.py
### This deliberately has no dependencies on Orbit Ribbon code; it's the ore.OREManager's job to turn this data into useful objects for the game

class Mesh:
	"""A mesh (collection of vertices and faces w/ normals) exported from the 3D editor.
	
	Data attributes:
	vertices - A sequence of (vertex tuple, normal vector tuple).
	faces - A sequence of (vi, vi, vi) tuples describing faces, where vi is an index into vertices.
	material - A string describing the name of the Material for this Mesh, or None if there is no Material.
	"""
	def __init__(self, vertices, faces, material):
		self.vertices = vertices
		self.faces = faces
		self.material = material
	

class Material:
	"""A material (surface appearance) exported from the 3D editor.
	
	Data attributes:
	dif_col, spe_col - The diffuse and specular colors, each as a 3-tuple.
	textures - A sequence of names of Texture objects associated with this Material.
	"""
	def __init__(self, dif_col, spe_col, textures):
		self.dif_col = dif_col
		self.spe_col = spe_col
		self.textures = textures


class Texture:
	"""A texture exported from the 3D editor.
	
	This is not the image itself, but a description of how that image is to be used.
	
	Data attributes:
	image - The name of the image to be used.
	"""
	def __init__(self, image):
		self.image = image


class Area:
	"""A description of an in-game location, exported from the 3D editor.
	
	Data attributes:
	objects - A sequence of area objects as (objname, meshname, position, rotmatrix).
	"""
	def __init__(self, objects):
		self.objects = objects


class Mission:
	"""A description of an in-game mission, which takes place in an Area but may add more objects.
	
	Data attributes:
	area_name - The Area that this mission takes place in.
	objects - A sequence of mission-specific objects as (objname, meshname, position, rotmatrix).
	"""
	def __init__(self, area_name, objects):
		self.area_name = area_name
		self.objects = objects
