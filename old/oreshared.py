from __future__ import division

### This is the common library connecting orbit-edit.py (which depends on the Blender API) and the rest of Orbit Ribbon via ore.py.
### It deliberately has no dependencies on Orbit Ribbon code; it's ore.OREManager's job to turn this data into useful objects for the game.


class Animation:
	"""A set of meshes forming an animation. Each mesh is 1/60th of a second advanced from the prior.
	
	Meshes that are part of animations are typically named MESHNAME-ANIMNAME-####, i.e. LIBAvatar-Run-0003
	
	Data attributes:
	frames - A sequence of meshname strings.
	"""
	def __init__(self, frames):
		self.frames = frames


class Mesh:
	# FIXME Need to put material properties like specularity here. First off, need to figure out materials/lighting model.
	"""A mesh (collection of vertices and faces w/ normals and texture data) exported from the 3D editor.
	
	Data attributes:
	facelists - A dictionary mapping image names to sequences of ((v, v, v), (n, n, n), (u, u, u)) tuples.
		Each facelist is textured by the Image namd, or not textured if the key is None.
		In the facelist, each v is a vertex point (3-tuple), n a normal vector (3-tuple), and u a texture coordinate (2-tuple) or None if no texture.
	"""
	def __init__(self, facelists):
		self.facelists = facelists


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
