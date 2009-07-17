#!/usr/bin/python

import profile, psyco, getopt

import app

PROFILING = False

app.ui_init()
app.sim_init()

if PROFILING:
	profile.run('app.run()', 'profile')
else:
	psyco.full() # TODO: Make sure this actually improves framerate
	app.run()
