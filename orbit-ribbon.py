#!/usr/bin/python

import profile

import app
import testobj

from geometry import *
from util import *

app.ui_init()
app.sim_init()

app.objects.append(testobj.Cube(Point(1.5, 0.0, -7.0)))

#profile.run('app.run()', 'profile')
app.run()

app.sim_deinit()
app.ui_deinit()
