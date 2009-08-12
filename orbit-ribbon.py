#!/usr/bin/python

import profile, getopt, sys, time

import app

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
			jump_area_name = areapart
		if len(missionpart) == 3 and missionpart[0] == "M" and missionpart[1:].isdigit():
			jump_mission_name = arg
	
	if jump_area_name is None or jump_mission_name is None:
		raise RuntimeError("Please supply a valid area-mission code. For example, A01-M02 for area 1, mission 2.")
	
	print "Jumping to area %s, mission %s" % (jump_area_name, jump_mission_name)


profiling = False
def opt_profile(arg):
	print "Profiling..."
	global profiling
	profiling = True


timing_init = False
def opt_timeini(arg):
	print "Timing initialization..."
	global timing_init
	timing_init = True


timing_run = False
def opt_timerun(arg):
	print "Timing run..."
	global timing_run
	timing_run = True


timing_report = False
def opt_timerep(arg):
	global timing_report
	timing_report = True


no_psyco = False
def opt_nopsyco(arg):
	global no_psyco
	no_psyco = True


# Command line options, each as a tuple of (callback function, short arg name, long arg name, help description)
options = (
	(opt_help,    "h",  "help",     "Displays a brief summary of command line options."),
	(opt_mission, "m:", "mission=", "Jumps straight to a mission. For example, run with \"-m A01-M02\" for area 1, mission 2."),
	(opt_profile, "p",  "profile",  "Generates python profiler output to the file 'profile' for this run."),
	(opt_timeini, "i",  "timeini",  "Times the initialization/loading process."),
	(opt_timerun, "t",  "timerun",  "Times the main frame loop. A summary will be printed after the game closes."),
	(opt_timerep, "r",  "timerep",  "Causes the -t option to also print a frame-by-frame timing list after the game closes."),
	(opt_nopsyco, "n",  "nopsyco",  "Disables the use of the performance optimizer psyco."),
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


if timing_init:
	start_time = time.time()
app.ui_init()
app.sim_init(timing = timing_run)
if jump_area_name is not None:
	app.init_area(jump_area_name)
if jump_mission_name is not None:
	app.init_mission(jump_mission_name)
if timing_init:
	print "Init time: %.2f seconds" % (time.time() - start_time)

if profiling:
	profile.run('app.run()', 'profile')
else:
	if not no_psyco:
		print "Loading psyco..."
		import psyco
		psyco.full()
	app.run()

if timing_run:
	totals = [0] * len(app.timing_names)
	for n, row in enumerate(app.timings):
		if timing_report or n == 0:
			if (n%20) == 0:
				for i in xrange(1, len(row)):
					print ("%8s" % app.timing_names[i]),
				print ("%8s" % "FPS"),
				print
		if len(row) == len(app.timing_names):
			for i in xrange(1, len(row)):
				section_ms = (row[i] - row[i-1])*1000
				totals[i-1] += section_ms
				if timing_report:
					print ("% 6.2fms" % section_ms),
			fps = 1/(row[-1] - row[0])
			totals[len(row)-1] += fps
			if timing_report:
				print ("% 8.4f" % fps),
				print
	
	if timing_report:
		print
		print "AVERAGES"
	for n, total in enumerate(totals):
		val = total/len(app.timings)
		if n == len(totals)-1:
			print ("% 8.4f" % val),
		else:
			print ("% 6.2fms" % val),
	print
