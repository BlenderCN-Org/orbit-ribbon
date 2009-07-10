#!/usr/bin/python

import profile, math, psyco

import app, testobj, colors, avatar, target, mission, sky, area, wfobj

from geometry import *
from util import *

app.ui_init()
app.sim_init()

app.mission_control = mission.MissionControl(
	win_cond_func = mission.AllRingsPassedFunction(),
	timer_start_func = mission.MinDistanceFunction(objs[0], objs[0].pos.__copy__(), 1.0)
)

#profile.run('app.run()', 'profile')
psyco.full()
app.run()
