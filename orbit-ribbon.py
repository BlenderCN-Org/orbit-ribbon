#!/usr/bin/python

import profile, psyco

import app

PROFILING = False

app.ui_init()
app.sim_init()

if PROFILING:
	profile.run('app.run()', 'profile')
else:
	psyco.full()
	app.run()
