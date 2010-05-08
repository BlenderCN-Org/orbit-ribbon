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

capsule_header_pattern = """
/*
%s: Automatically generated header from source file %s.

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
*/

#ifndef %s
#define %s

const char* %s =
""" 

import os

def build_capsulate(source, target, env):
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
	
	matches = {}
	reverse_matches = {}
	for s in src_list:
		if not os.path.isfile(s):
			raise RuntimeError("build_capsulate: Unable to find source file at %s" % s)
		tgt = os.path.basename(s) + ".h"
		
		found_match = False
		for t in tgt_list:
			if os.path.basename(t) == os.path.basename(s) + ".h":
				matches[s] = t
				reverse_matches[t] = s
				found_match = True
				break
		
		if not found_match:
			raise RuntimeError("build_capsulate: Source file %s has no matching header in the target list" % s)
	
	if len(matches) != len(reverse_matches):
		raise RuntimeError("build_capsulate: Not a one-to-one correspondence between source and target files")
	
	def capsulize(src, tgt):
		sym_name = "CAPSULE_" + os.path.basename(src).replace(".", "_").upper()
		guard_name = "ORBIT_RIBBON_" + sym_name + "_H";
		
		input_fh = open(src)
		fh = open(tgt, "w")
		fh.write(capsule_header_pattern % (os.path.basename(tgt), os.path.basename(src), guard_name, guard_name, sym_name))
		first_line = True
		for line in input_fh:
			if first_line:
				first_line = False
			else:
				fh.write("\n");
			fh.write('"' + line.replace('\\', '\\\\').replace('"', '\\"').replace("\n", "") + '\\n"')
		fh.write(";\n\n#endif\n")
		
		input_fh.close()
		fh.close()
		return 0
		
	for s in src_list:
		if Execute(Action(lambda target, source, env: capsulize(s, matches[s]), "Encapsulating %s into header %s" % (str(s), str(matches[s])))):
			raise RuntimeError("build_capsulate: construction failed")


def build_xsd(mode, source, target, env):
	args = [mode]
	args.extend(["--guard-prefix", "ORBIT_RIBBON_AUTOXSD"])
	args.extend(["--hxx-suffix", ".h"])
	args.extend(["--cxx-suffix", ".cpp"])
	
	if mode == 'cxx-tree':
		args.extend(["--type-naming", "ucc"])
		args.extend(["--generate-doxygen"])
		args.extend(["--generate-serialization"])
		args.extend(["--generate-polymorphic"])
		args.extend(["--root-element-all"])
	elif mode == 'cxx-parser':
		args.extend(["--type-map", "xml/types.map"])
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
	action = Action(lambda source, target, env: build_xsd('cxx-tree', source, target, env), "C++/Tree for XML schemas: $SOURCES"),
	suffix = {'.xsd' : '.cpp'},
	emitter = xsd_emitter
)
env['BUILDERS']['XSDParser'] = Builder(
	action = Action(lambda source, target, env: build_xsd('cxx-parser', source, target, env), "C++/Parser for XML schemas: $SOURCES"),
	suffix = {'.xsd' : '-pskel.cpp'},
	emitter = xsd_emitter
)
env['BUILDERS']['Capsulate'] = Builder(
	action = Action(build_capsulate, "Encapsulating to headers: $SOURCES"),
	suffix = {'.xsd' : '.h', '.xml' : '.h'}
)

tree_xsds = ['orepkgdesc', 'save']
parser_xsds = ['oreanim']
capsulated_files = Glob('xml/*.xsd', strings = True) + Glob('xml/*.xml', strings = True)
cpp_gen_dir = 'cpp/autoxsd/'

tree_built = env.XSDTree(
	['%s/%s.cpp' % (cpp_gen_dir, n) for n in tree_xsds],
	['xml/%s.xsd' % n for n in tree_xsds]
)
parser_built = env.XSDParser(
	['%s/%s-pskel.cpp' % (cpp_gen_dir, n) for n in parser_xsds],
	['xml/%s.xsd' % n for n in parser_xsds]
)
capsulate_built = env.Capsulate(
	['%s/%s.h' % (cpp_gen_dir, os.path.basename(str(n))) for n in capsulated_files],
	capsulated_files
)
env.Program(
	'orbit-ribbon',
	[b for b in (tree_built + parser_built) if str(b).endswith(".cpp")] + Glob('buildtmp/*.cpp'),
	LIBS=['GL', 'GLU', 'GLEW', 'ode', 'SDL', 'SDL_image', 'zzip', 'boost_filesystem-mt', 'boost_regex-mt', 'boost_program_options-mt', 'xerces-c', 'ftgl'],
	CCFLAGS='-Wall -Wextra -pedantic-errors -DdDOUBLE -I/usr/include/freetype2/'
)
