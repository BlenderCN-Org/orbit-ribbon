#!/usr/bin/python

import profile, psyco, getopt, sys

import app

PROFILING = False

def opt_help(arg):
	# TODO Should probably print copyright/license info here
	print
	print "Orbit Ribbon, Version: %s" % app.VERSION
	print
	print "Command line arguments:"
	for func, short, long, desc in options:
		if short.endswith(":"):
			short = short[:-1]
		if long.endswith("="):
			long = long[:-1]
		print "-%s or --%s" % (short, long)
		print "    %s" % desc
	print
	sys.exit()

jump_area_name = None
jump_mission_name = None
def opt_mission(arg):
	global jump_area_name, jump_mission_name
	arg = arg.upper().strip()
	parts = arg.split("-")
	if len(parts) == 2:
		areapart = parts[0]
		missionpart = parts[1]
		if len(areapart) == 3 and areapart[0] == "A" and areapart[1:].isdigit():
			jump_area_name = areapart + "-Base"
		if len(missionpart) == 3 and missionpart[0] == "M" and missionpart[1:].isdigit():
			jump_mission_name = arg
	
	if jump_area_name is None or jump_mission_name is None:
		raise RuntimeError("Please supply a valid area-mission code. For example, A01-M02 for area 1, mission 2.")


# Command line options, each as a tuple of (callback function, short arg name, long arg name, help description)
options = (
	(opt_help,    "h",  "help",     "Displays a brief summary of command line options."),
	(opt_mission, "m:", "mission=", "Jumps straight to a mission. For example, run with \"-m A01-M02\" for area 1, mission 2."),
)

opt_results, other_args = getopt.gnu_getopt(
	args = sys.argv[1:],
	shortopts = "".join([x[1] for x in options]),
	longopts = [x[2] for x in options]
)
for opt, val in opt_results:
	for func, short, long, desc in options:
		if short.endswith(":"):
			short = short[:-1]
		if long.endswith("="):
			long = long[:-1]
		if opt == ("-" + short) or opt == ("--" + long):
			func(val)


# Magic begins here

app.ui_init()
app.sim_init()
if jump_area_name is not None:
	app.init_area(jump_area_name)
if jump_mission_name is not None:
	app.init_mission(jump_mission_name)

if PROFILING:
	profile.run('app.run()', 'profile')
else:
	psyco.full() # TODO: Make sure this actually improves framerate
	app.run()
