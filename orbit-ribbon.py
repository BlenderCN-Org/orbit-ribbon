#!/usr/bin/python

import profile

import app
import testobj

from geometry import *
from util import *

app.ui_init()
app.sim_init()

o = testobj.Cube(Point(2.0, 0.0, -7.0))
o.body.addForce((-4, 1, 0))
app.objects.append(o)

#profile.run('app.run()', 'profile')
app.run()

app.sim_deinit()
app.ui_deinit()
