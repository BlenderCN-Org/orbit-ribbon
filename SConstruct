"""
SConstruct: Build script for Orbit Ribbon.

Copyright 2009 David Simon. You can reach me at david.mike.simon@gmail.com

This file is part of Orbit Ribbon.

Orbit Ribbon is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Orbit Ribbon is distributed in the hope that it will be awesome,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Orbit Ribbon.  If not, see http://www.gnu.org/licenses/
"""

VariantDir('buildtmp', 'cpp', duplicate=0)
env = Environment()
env.Program('orbit-ribbon', Glob('buildtmp/*.cpp') + Glob('buildtmp/autoxsd/*.cpp'), LIBS=['GL', 'GLU', 'ode', 'SDL', 'SDL_mixer', 'SDL_image', 'zzip', 'boost_filesystem-mt', 'xerces-c'], CCFLAGS='-Wall -Wextra -pedantic-errors')
