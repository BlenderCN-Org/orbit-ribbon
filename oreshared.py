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
	vertices - A sequence of (vertex tuple, normal vector tuple) tuples describing vertices.
	uvpoints - A sequence of 2-tuples describing UV texture coordinate positions.
	images - A sequence of strings, each a name of an Image used for this Mesh's texture.
	faces - A sequence of ((vi, vi, vi), ii, (ui, ui, ui)) tuples describing faces.
		In the above, vi is an index into vertices, ii an index into images, and ui an index into uvpoints.
		If the face has no image, ii and ui values will all be None.
	"""
	def __init__(self, vertices, uvpoints, images, faces):
		self.vertices = vertices
		self.uvpoints = uvpoints
		self.images = images
		self.faces = faces


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
