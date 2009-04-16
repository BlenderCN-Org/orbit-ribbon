#!/usr/bin/python

import profile

import app, testobj, colors, avatar, target

from geometry import *
from util import *

app.ui_init()
app.sim_init()

app.objects.append(
	avatar.Avatar(Point(0.0, 0.0, 0.0))
)

app.objects.append(
	testobj.Cube(Point(0.0, 0.0, 8.0), color = colors.green)
)

app.objects.append(
	testobj.Ground(Point(0.0, -8.0, 0.0))
)

app.objects.append(
	target.Ring(Point(4.0, 15.0, 30.0))
)

app.objects.append(
	target.Ring(Point(-4.0, 15.0, 60.0))
)

app.objects.append(
	target.Ring(Point(4.0, -25.0, 40.0))
)

app.objects.append(
	target.Ring(Point(-4.0, -25.0, -20.0))
)

#profile.run('app.run()', 'profile')
app.run()

app.sim_deinit()
app.ui_deinit()
