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

import bpy
from io_utils import ExportHelper
import os

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
    print("Writing ORE %s" % self.filepath)
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
