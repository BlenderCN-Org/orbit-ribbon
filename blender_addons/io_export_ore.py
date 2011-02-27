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

OREANIM_NAMESPACE = "http://www.orbit-ribbon.org/OREAnim1"
OREANIM_NS_PREFIX = "{%s}" % OREANIM_NAMESPACE
OREANIM_NSMAP = {"oreanim" : OREANIM_NAMESPACE}

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

    # Create a new ORE file; just a regular zipfile
    zfh = zipfile.ZipFile(self.filepath, mode = "w", compression = zipfile.ZIP_DEFLATED)
    zfh.writestr("ore-version", "1") # ORE file format version (this will be 1 until the first backwards-incompatible change after release)
    zfh.writestr("ore-name", descDoc.xpath("niceName/text()")[0]) # The nice name of the ORE package, separate so that we can get it without XML

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
