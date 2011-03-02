#
# Copyright 2011 David Simon <david.mike.simon@gmail.com>
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be awesome,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/
#

bl_addon_info = {
  "name": "Export ORE (Orbit Ribbon Episode)",
  "author": "David Mike Simon",
  "version": (0, 1),
  "blender": (2, 5, 6),
  "api": 33713,
  "location": "File > Export",
  "description": "Export to the Orbit Ribbon Episode format",
  "warning": "",
  "category": "Import-Export"
}

import bpy, mathutils
from io_utils import ExportHelper
import os, re, sys, zipfile, datetime, xml.dom.minidom
from math import *

MATRIX_BLEN2ORE = mathutils.Matrix.Rotation(-90, 4, 'X')
MATRIX_INV_BLEN2ORE = MATRIX_BLEN2ORE.copy().invert()

OREPKG_NS = "http://www.orbit-ribbon.org/ORE1"
OREANIM_NS = "http://www.orbit-ribbon.org/OREAnim1"
SCHEMA_NS = "http://www.w3.org/2001/XMLSchema-instance"

def fixcoords(t): # Given a 3-sequence, returns it so that axes changed to fit OpenGL standards (y is up, z is forward)
  t = mathutils.Vector(t) * MATRIX_BLEN2ORE
  return (t[0], t[1], t[2])

def genrotmatrix(x, y, z): # Returns a 9-tuple for a column-major 3x3 rotation matrix with axes corrected ala fixcoords
  m = (
    MATRIX_INV_BLEN2ORE *
    mathutils.Matrix.Rotation(radians(x), 4, 'X') *
    mathutils.Matrix.Rotation(radians(y), 4, 'Y') *
    mathutils.Matrix.Rotation(radians(z), 4, 'Z') *
    MATRIX_BLEN2ORE
  )
  return ( # Elide final column and row
    m[0][0], m[0][1], m[0][2],
    m[1][0], m[1][1], m[1][2],
    m[2][0], m[2][1], m[2][2],
  )

def populateMeshNode(meshNode, mesh, doc):
    imgs = set()
    faceCount = 0

    imgName = None
    try:
      imgName = mesh.uv_textures[0].data[0].image.filepath
    except IndexError:
      pass # This mesh has no uv textures
    else:
      imgName = re.sub(r"^//", "", imgName)
      meshNode.setAttribute("texture", os.path.basename(imgName))

    # For each face, add it to the meshNode and add to the vertex list any vertices it has that are new.
    # We have to do it this way because a given vertex might have several different UV values, depending on which face
    # it's on.
    vertices = {} # Map of (pos, normal, uv) tuple to vertex index
    for fOffset, f in enumerate(mesh.faces):
      offsets = [(0,1,2)]
      if len(f.vertices) == 4:
        # Treat each quad as though it were two triangles
        offsets.append((0,2,3))
      for offsetGroup in offsets:
        vertIndexes = []
        for offset in offsetGroup:
          vertex = mesh.vertices[f.vertices[offset]]
          uv = (0.0, 0.0)
          if imgName:
            uv = mesh.uv_textures[0].data[fOffset].uv[offset]
          vertKey = (tuple(vertex.co), tuple(vertex.normal), tuple(uv))
          vertIdx = None
          if vertKey in vertices:
            # Another face has already referenced this vertex+uv
            vertIdx = vertices[vertKey]
          else:
            # A new vertex+uv
            vertIdx = len(vertices)
            vertices[vertKey] = vertIdx
          vertIndexes.append(vertIdx)
        fNode = doc.createElementNS(OREPKG_NS, "f")
        fNode.appendChild(doc.createTextNode(" ".join([str(idx) for idx in vertIndexes])))
        meshNode.appendChild(fNode)
        faceCount += 1

    # Load the vertices into the node
    vertexList = list(vertices.items())
    vertexList.sort(key = lambda i: i[1])
    for ((p, n, t), idx) in vertexList:
      vxNode = doc.createElementNS(OREPKG_NS, "v")
      ptNode = doc.createElementNS(OREPKG_NS, "p")
      ptNode.appendChild(doc.createTextNode(" ".join([str(x) for x in fixcoords(p)])))
      vxNode.appendChild(ptNode)
      nmNode = doc.createElementNS(OREPKG_NS, "n")
      nmNode.appendChild(doc.createTextNode(" ".join([str(x) for x in fixcoords(n)])))
      vxNode.appendChild(nmNode)
      txNode = doc.createElementNS(OREPKG_NS, "t")
      txNode.appendChild(doc.createTextNode(" ".join([str(x) for x in t])))
      vxNode.appendChild(txNode)
      meshNode.appendChild(vxNode)

    # Set the attributes that let the loader know in advance how much space to allocate
    meshNode.setAttribute("vertcount", str(len(vertexList)))
    meshNode.setAttribute("facecount", str(faceCount))

    return imgName

def include_image(zfh, path):
  zfh.write(os.path.join(os.path.dirname(bpy.data.filepath), path), "image-%s" % os.path.basename(path), zipfile.ZIP_STORED)

class Export_Ore(bpy.types.Operator, ExportHelper):
  """Exports all scenes as an Orbit Ribbon Episode file."""
  filetype_desc = "Orbit Ribbon Episode (.ore)"
  filename_ext = ".ore"
  bl_idname = "export_ore"
  bl_label = "Export ORE"

  @classmethod
  def poll(cls, context):
    return True

  def execute(self, context):
    # Load the description from the oredesc text datablock
    descDoc = None
    if not "oredesc" in bpy.data.texts.keys():
      self.report({'ERROR'}, "Couldn't find the oredesc text")
      return {'CANCELLED'}
    try:
      descDoc = xml.dom.minidom.parseString(bpy.data.texts["oredesc"].as_string())
    except Exception as e:
      self.report({'ERROR'}, "Parsing error: %s" % e)
      return {'CANCELLED'}

    # Try to find the pkgDesc root node
    pkgDesc = None
    for child in descDoc.childNodes:
      if child.localName == 'pkgDesc':
        pkgDesc = child
        break
    if pkgDesc is None:
      self.report({'ERROR'}, "Document has no pkgDesc element")
      return {'CANCELLED'}

    # Create a new ORE file; just a regular zipfile
    zfh = zipfile.ZipFile(self.filepath, mode = "w", compression = zipfile.ZIP_DEFLATED)
    zfh.writestr("ore-version", "1") # ORE file format version (this will be 1 until the first backwards-incompatible change after release)

    # Write the nice name of the ORE package, separate so that we can get it without XML
    niceName = None
    for child in pkgDesc.childNodes:
      if child.localName == 'niceName':
        niceName = child.childNodes[0].nodeValue
        break
    if niceName is None:
      self.report({'ERROR'}, "Pkg desc has no niceName element")
      return {'CANCELLED'}
    zfh.writestr("ore-name", niceName)

    # Find all the xml elements that correspond to area and mission scenes
    sceneNodes = []
    for child in pkgDesc.childNodes:
      if child.localName != 'area':
        continue
      sceneNodes.append(child)
      for subchild in child.childNodes:
        if subchild.localName != 'mission':
          continue
        sceneNodes.append(subchild)

    # Add scene information to the desc document
    for node in sceneNodes:
      if not node.hasAttribute("sceneName"):
        self.report({'ERROR'}, "Area or mission has no sceneName!")
        return {'CANCELLED'}
      sceneName = node.getAttribute("sceneName")
      if not sceneName in bpy.data.scenes.keys():
        self.report({'ERROR'}, "Couldn't find any scene named %s" % sceneName)
        return {'CANCELLED'}
      for obj in bpy.data.scenes[sceneName].objects:
        if obj.type == "MESH" or obj.type == "SURFACE":
          try:
            objNode = descDoc.createElementNS(OREPKG_NS, "obj")
            objNode.setAttribute("objName", obj.name)
            objNode.setAttribute("dataName", obj.data.name)
            node.appendChild(objNode)

            posNode = descDoc.createElementNS(OREPKG_NS, "pos")
            posNode.appendChild(descDoc.createTextNode(" ".join([str(x) for x in fixcoords(obj.location)])))
            objNode.appendChild(posNode)

            rotNode = descDoc.createElementNS(OREPKG_NS, "rot")
            rotNode.appendChild(descDoc.createTextNode(" ".join([str(x) for x in genrotmatrix(*obj.rotation_euler)])))
            objNode.appendChild(rotNode)

            libDataMatch = re.match(r"LIB(.+?)(?:\.\d+)?$", obj.data.name)
            if obj.type == "SURFACE":
              # At the moment, the only thing surfaces are used for are bubbles
              objNode.setAttribute("implName", "Bubble")
              objNode.setAttributeNS(SCHEMA_NS, "type", "{%s}BubbleObjType" % OREPKG_NS)
              radNode = descDoc.createElementNS(OREPKG_NS, "radius")
              radNode.appendChild(descDoc.createTextNode(str((sum(obj.dimensions)/3))))
              objNode.appendChild(radNode)
            elif libDataMatch:
              # If it's a library object, have the game try to find a matching implementation
              # The default mesh object implementation will be used if this doesn't match
              objNode.setAttribute("implName", libDataMatch.group(1))
          except Exception as e:
            self.report({'ERROR'}, "Problem exporting object %s: %s" % (obj.name, e))
            return {'CANCELLED'}

    zfh.writestr("ore-desc", descDoc.toxml(encoding='UTF-8'))

    include_image(zfh, "images/cursor.png")
    include_image(zfh, "images/star.png")
    include_image(zfh, "images/title.png")
    include_image(zfh, "images/starmap-1.png")
    include_image(zfh, "images/starmap-2.png")
    include_image(zfh, "images/starmap-3.png")
    include_image(zfh, "images/starmap-4.png")
    include_image(zfh, "images/starmap-5.png")
    include_image(zfh, "images/starmap-6.png")

    copiedImages = set() # Set of image names that have already been copied into the zipfile

    for mesh in bpy.data.meshes:
      # Export each simple mesh as a 1-frame animation
      animDoc = xml.dom.minidom.Document()
      animNode = animDoc.createElementNS(OREANIM_NS, "animation")
      animNode.setAttribute("name", mesh.name)
      animDoc.appendChild(animNode)
      meshNode = animDoc.createElementNS(OREANIM_NS, "frame")
      imgName = populateMeshNode(meshNode, mesh, animDoc)
      animNode.appendChild(meshNode)
      if imgName is not None:
        copiedImages.add(imgName)
      zfh.writestr("mesh-%s" % mesh.name, animDoc.toxml(encoding='UTF-8'))

    for img in copiedImages:
      include_image(zfh, img)

    return {'FINISHED'}

  def invoke(self, context, event):
    context.window_manager.fileselect_add(self)
    return {'RUNNING_MODAL'}


#### REGISTER

def menu_func(self, context):
  op = self.layout.operator(Export_Ore.bl_idname, text=Export_Ore.filetype_desc)
  op.filepath = os.path.splitext(bpy.data.filepath)[0] + ".ore" # Default save path

def register():
  bpy.types.INFO_MT_file_export.append(menu_func)

def unregister():
  bpy.types.INFO_MT_file_export.remove(menu_func)

if __name__ == "__main__":
  register()
