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
	
	if len(src_list)*2 != len(tgt_list):
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
		for ext in ("cpp", "h"):
			if (tgt_base + "." + ext) not in [os.path.basename(t) for t in tgt_list]:
				raise RuntimeError("build_xsd: No file with extension %s in target list corresponding with source file %s" % (ext, s))
		args.append(s)
	
	if Execute(Action([["xsdcxx"] + args])):
		raise RuntimeError("build_xsd: xsdcxx call failed")
	for t in tgt_list:
		tmpfn = t + ".tmp"
		if Execute(Action([["sed", "-n", "s/long long/long/; w %s" % tmpfn, t]])):
			raise RuntimeError("build_xsd: sed call failed")
		if Execute(Move(t, tmpfn)):
			raise RuntimeError("build_xsd: post-sed move failed")


def xsd_emitter(target, source, env):
	added = []
	target_strs = [str(t) for t in target]
	for t in target_strs:
		if t.endswith(".cpp"):
			header = t[:-4] + ".h"
			if header not in target_strs:
				added.append(header)
	return (target + added, source)


VariantDir('buildtmp', 'cpp', duplicate=0)
env = Environment()

env['BUILDERS']['XSDTree'] = Builder(
	action = lambda source, target, env: build_xsd('cxx-tree', source, target, env),
	suffix = {'.xsd' : '.cpp'},
	emitter = xsd_emitter
)
env['BUILDERS']['XSDParser'] = Builder(
	action = lambda source, target, env: build_xsd('cxx-parser', source, target, env),
	suffix = {'.xsd' : '-pskel.cpp'},
	emitter = xsd_emitter
)

tree_xsds = ['orepkgdesc']
parser_xsds = ['oreanimation', 'oremeshtype', 'oremesh']
xsd_cpp_dir = 'cpp/autoxsd/'

tree_built = env.XSDTree(
	['%s/%s.cpp' % (xsd_cpp_dir, n) for n in tree_xsds],
	['xml/%s.xsd' % n for n in tree_xsds]
)
parser_built = env.XSDParser(
	['%s/%s-pskel.cpp' % (xsd_cpp_dir, n) for n in parser_xsds],
	['xml/%s.xsd' % n for n in parser_xsds]
)
env.Program(
	'orbit-ribbon',
	[b for b in (tree_built + parser_built) if str(b).endswith(".cpp")] + Glob('buildtmp/*.cpp'),
	LIBS=['GL', 'GLU', 'ode', 'SDL', 'SDL_mixer', 'SDL_image', 'zzip', 'boost_filesystem-mt', 'xerces-c'],
	CCFLAGS='-Wall -Wextra -pedantic-errors'
)
