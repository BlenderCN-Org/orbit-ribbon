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

import os

VariantDir('buildtmp', 'cpp', duplicate=0)
env = Environment()

def build_xsd(mode, source, target, env):
	args = [mode]
	args.extend(["--guard-prefix", "ORBIT_RIBBON_AUTOXSD"])
	args.extend(["--hxx-suffix", ".h"])
	args.extend(["--cxx-suffix", ".cpp"])
	
	if mode == 'cxx-tree':
		args.extend(["--type-naming", "ucc"])
		args.extend(["--generate-doxygen"])
		args.extend(["--generate-serialization"])
	elif mode == 'cxx-parser':
		pass
	else:
		raise RuntimeError("build_xsd: Unknown mode %s" % mode)
		
	src_list = []
	if isinstance(source, list):
		src_list = [str(s) for s in source]
	else:
		src_list = [str(source)]
	
	tgt_list = []
	if isinstance(target, list):
		tgt_list = [str(t) for t in target]
	else:
		tgt_list = [str(target)]
	
	if len(src_list) != len(tgt_list):
		raise RuntimeError("build_xsd: Source and target lists mismatched on length")
	
	output_dir = None
	for t in tgt_list:
		if output_dir == None:
			output_dir = os.path.dirname(t)
		elif output_dir != os.path.dirname(t):
			raise RuntimeError("build_xsd: Output files not all in the same directory")
	args.extend(["--output-dir", output_dir])
	
	for s in src_list:
		if not s.endswith(".xsd"):
			raise RuntimeError("build_xsd: Unknown extension on source file %s" % s)
		if not os.path.isfile(s):
			raise RuntimeError("build_xsd: Unable to find source file at %s" % s)
		tgt_base = os.path.basename(s)[:-4]
		if mode == 'cxx-parser':
			tgt_base += "-pskel"
		if (tgt_base + ".cpp") not in [os.path.basename(t) for t in tgt_list]:
			raise RuntimeError("build_xsd: No cpp file in target list corresponding with source file %s" % s)
		args.append(s)
	
	if Execute(Action([["xsdcxx"] + args])):
		raise RuntimeError("build_xsd: xsdcxx call failed")
	for t in tgt_list:
		for fn in (t, t[:-4] + ".h"):
			tmpfn = fn + ".tmp"
			if Execute(Action([["sed", "-n", "s/long long/long/; w %s" % tmpfn, fn]])):
				raise RuntimeError("build_xsd: sed call failed")
			if Execute(Move(fn, tmpfn)):
				raise RuntimeError("build_xsd: post-sed move failed")


env['BUILDERS']['XSDTree'] = Builder(
	action = lambda source, target, env: build_xsd('cxx-tree', source, target, env),
	suffix = {'.xsd' : '.cpp'}
)
env['BUILDERS']['XSDParser'] = Builder(
	action = lambda source, target, env: build_xsd('cxx-parser', source, target, env),
	suffix = {'.xsd' : '-pskel.cpp'}
)

tree_built = env.XSDTree(
	['cpp/autoxsd/orepkgdesc.cpp'],
	['xml/orepkgdesc.xsd']
)
parser_built = env.XSDParser(
	['cpp/autoxsd/oreanimation-pskel.cpp', 'cpp/autoxsd/oremeshtype-pskel.cpp', 'cpp/autoxsd/oremesh-pskel.cpp'],
	['xml/oreanimation.xsd', 'xml/oremeshtype.xsd', 'xml/oremesh.xsd']
)
env.Program(
	'orbit-ribbon',
	tree_built + parser_built + Glob('buildtmp/*.cpp'),
	LIBS=['GL', 'GLU', 'ode', 'SDL', 'SDL_mixer', 'SDL_image', 'zzip', 'boost_filesystem-mt', 'xerces-c'],
	CCFLAGS='-Wall -Wextra -pedantic-errors'
)
