#!/usr/bin/python

import profile

import app
import gameobj
import colors
import consenv
import collision

from geometry import *
from util import *

app.ui_init()
app.sim_init()

# Insert objects here

#profile.run('app.run()', 'profile')
app.run()

app.sim_deinit()
app.ui_deinit()
