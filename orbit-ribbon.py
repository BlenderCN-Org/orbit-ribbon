#!/usr/bin/python

import profile, math

import app, testobj, colors, avatar, target, mission, sky

from geometry import *
from util import *

app.ui_init()
app.sim_init()

app.objects.append(
	avatar.Avatar(Point(4.0, 12.0, -35.0))
)

app.objects.append(
	testobj.Cube(Point(-20.0, 4.0, 8.0), color = colors.green)
)
app.objects[-1].body.addForce((50.0, 10.0, -10.0))

app.objects.append(
	testobj.Cube(Point(-23.0, 4.0, 8.0), color = colors.green)
)
app.objects[-1].body.addForce((50.0, 0.0, -10.0))

app.objects.append(
	testobj.Cube(Point(-20.0, 7.0, 6.0), color = colors.green)
)
app.objects[-1].body.addForce((40.0, 3.0, -5.0))

app.objects.append(
	testobj.Cube(Point(-23.0, 5.0, 8.0), color = colors.green)
)
app.objects[-1].body.addForce((45.0, 2.0, 5.0))

app.objects.append(
	testobj.Cube(Point(-18.0, 2.0, 10.0), color = colors.green)
)
app.objects[-1].body.addForce((55.0, -2.0, -10.0))

app.objects.append(
	testobj.Cube(Point(-20.0, 10.0, 8.0), color = colors.green)
)
app.objects[-1].body.addForce((50.0, -7.0, 0.0))

app.objects.append(
	testobj.Ground(Point(0.0, -8.0, 0.0))
)

app.objects.append(
	target.Ring(Point(4.0, 15.0, 30.0))
)

app.objects.append(
	target.Ring(Point(-4.0, 17.0, 60.0))
)

app.objects.append(
	target.Ring(Point(4.0, -25.0, 40.0))
)

app.objects.append(
	target.Ring(Point(-4.0, -23.0, -20.0))
)

app.mission_control = mission.MissionControl(
	win_cond_func = mission.AllRingsPassedFunction(),
	timer_start_func = mission.MinDistanceFunction(app.objects[0], app.objects[0].pos.__copy__(), 1.0)
)

#app.sky_stuff = sky.SkyStuff(
#	game_angle = 0.17,
#	game_y_offset = 1100,
#	game_d_offset = 800,
#	game_tilt = (67, 0.4, 0, 0.7),
#	t3_angle = 0.8,
#)

app.sky_stuff = sky.SkyStuff()

#profile.run('app.run()', 'profile')
app.run()

app.sim_deinit()
app.ui_deinit()
