import Blender, os
from math import *

WORKING_DIR = os.path.dirname(Blender.Get("filename"))

def do_export():
	print "Exporting..."
	Blender.Draw.PupMenu("Exported just fine%t|OK, thanks a bunch")

def menu():
	menu = ("Add GameObject", "Export")
	r = Blender.Draw.PupMenu("Orbit Ribbon Level Editing%t|" + "|".join(["%s%%x%u" % (name, num) for num, name in enumerate(menu)]))
	if r >= 0:
		if menu[r] == "Export":
			do_export()
	Blender.Redraw()

menu()
