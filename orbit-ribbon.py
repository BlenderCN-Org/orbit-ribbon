#!/usr/bin/python

import profile

import app, testobj, colors, avatar, target

from geometry import *
from util import *

app.ui_init()
app.sim_init()

o = avatar.Avatar(Point(0.0, 0.0, 0.0))
app.objects.append(o)

o = testobj.Cube(Point(0.0, 0.0, 8.0), color = colors.green)
app.objects.append(o)

o = testobj.Ground(Point(0.0, -8.0, 0.0))
app.objects.append(o)

o = target.Ring(Point(4.0, 15.0, 35.0))
app.objects.append(o)

#profile.run('app.run()', 'profile')
app.run()

app.sim_deinit()
app.ui_deinit()
