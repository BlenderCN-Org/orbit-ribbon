#!/usr/bin/python

import profile, math, psyco

import app, testobj, colors, avatar, target, mission, sky, area, wfobj

from geometry import *
from util import *

app.ui_init()
app.sim_init()

objs = []

objs.append(
	avatar.Avatar(Point(4.0, 12.0, -35.0))
)

objs.append(
	testobj.Cube(Point(-20.0, 4.0, 8.0), color = colors.green)
)
objs[-1].body.addForce((50.0, 10.0, -10.0))

objs.append(
	testobj.Cube(Point(-23.0, 4.0, 8.0), color = colors.green)
)
objs[-1].body.addForce((50.0, 0.0, -10.0))

objs.append(
	testobj.Cube(Point(-20.0, 7.0, 6.0), color = colors.green)
)
objs[-1].body.addForce((40.0, 3.0, -5.0))

objs.append(
	testobj.Cube(Point(-23.0, 5.0, 8.0), color = colors.green)
)
objs[-1].body.addForce((45.0, 2.0, 5.0))

objs.append(
	testobj.Cube(Point(-18.0, 2.0, 10.0), color = colors.green)
)
objs[-1].body.addForce((55.0, -2.0, -10.0))

objs.append(
	testobj.Cube(Point(-20.0, 10.0, 8.0), color = colors.green)
)
objs[-1].body.addForce((50.0, -7.0, 0.0))

objs.append(
	testobj.Ground(Point(0.0, -8.0, 0.0), 50, 100)
)

objs.append(
	target.Ring(Point(4.0, 15.0, 30.0))
)

objs.append(
	target.Ring(Point(-4.0, 17.0, 60.0))
)

objs.append(
	target.Ring(Point(4.0, -25.0, 40.0))
)

objs.append(
	target.Ring(Point(-4.0, -23.0, -20.0))
)

objs.append(
	wfobj.WFObj("exportdata/jungle.obj", Point(0.0, 0.0, 300))
)

objs.append(
	testobj.GreenSphere(Point(-15000, 5000, 15000))
)

sky_stuff = sky.SkyStuff(
	game_angle = 0.17,
	game_y_offset = 1100,
	game_d_offset = 800,
	game_tilt = (67, 0.4, 0, 0.7),
	t3_angle = 0.8,
)

app.areas.append(
	area.AreaDesc(
		sky_stuff = sky_stuff,
		objects = objs
	)
)

app.mission_control = mission.MissionControl(
	win_cond_func = mission.AllRingsPassedFunction(),
	timer_start_func = mission.MinDistanceFunction(objs[0], objs[0].pos.__copy__(), 1.0)
)

#profile.run('app.run()', 'profile')
psyco.full()
app.run()
