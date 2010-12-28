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

ORBIT_RIBBON_VERSION = "prealpha"

preamble_pattern = """/*
%s: %s.

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

""" 

import os, Image, cStringIO, lxml.etree, datetime, commands


env = Environment(ENV = {'PATH' : os.environ['PATH']}, tools = ['mingw'])
# FIXME: Later need to figure out how to compile only minizip and autoxsd with lenient options
# Then, can disable the "long long" fix for xsd
#CCFLAGS = '-Wall -Wextra -pedantic-errors -DdDOUBLE'
CCFLAGS = '-Wall -DdDOUBLE'
if 'win' in env['HOST_OS']:
  CCFLAGS += ' -DIN_WINDOWS'

debug = ARGUMENTS.get('debug', 0)
if int(debug):
  CCFLAGS += ' -g'

def pow2le(n):
  r = 1
  while r < n:
    r *= 2
  return r

def img_to_png_char_array(img):
  sio = cStringIO.StringIO()
  img.save(sio, 'png')
  sio.seek(0)
  return [str(ord(c)) for c in sio.read()]

def write_c_string(fh, name, s):
  fh.write("const char* %s =\n" % name)
  for line in s:
    fh.write('"' + line.replace('\\', '\\\\').replace('"', '\\"').replace("\n", "") + '\\n"')
    fh.write("\n")
  fh.write(";\n")

def file_sub(fn, s, t):
  contents = []
  fh = file(fn)
  for line in fh:
    contents.append(line.replace(s, t))
  fh.close()
  fh = file(fn, "w")
  for line in contents:
    fh.write(line)

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
    fh.write(preamble_pattern % (os.path.basename(tgt), "Automatically generated from %s" % os.path.basename(src)))
    fh.write("#ifndef %s\n" % guard_name)
    fh.write("#define %s\n" % guard_name)
    fh.write("\n")
    write_c_string(fh, sym_name, input_fh)
    fh.write("\n#endif\n")

    input_fh.close()
    fh.close()
    return 0

  for s in src_list:
    if env.Execute(env.Action(lambda target, source, env: capsulize(s, matches[s]), "%s encapsulated into header %s" % (str(s), str(matches[s])))):
      raise RuntimeError("build_capsulate: construction failed")


FONT_GLYPHS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*()-=_+`~[]{}\\|;':,.<>/?"
def build_font(source, target, env):
  fonts_to_build = {}
  for s in source:
    f = os.path.basename(os.path.dirname(str(s)))
    if f not in fonts_to_build:
      fonts_to_build[f] = []
    fonts_to_build[f].append(s)

  font_desc_schema = lxml.etree.XMLSchema(lxml.etree.parse(os.path.join("xml", "fontdesc.xsd")))

  for f, srclist in fonts_to_build.iteritems():
    print "Generating fontdata %s from %s" % (f, ",".join([str(s) for s in srclist]))

    # TODO: Figure out how to set the namespace properly, the ns0 stuff lxml generates as-is is kind of ugly
    desc_doc = lxml.etree.Element("{http://www.orbit-ribbon.org/ORFontDesc}fontdesc")

    srcimgs = []
    for s in srclist:
      img = Image.open(str(s))
      srcimgs.append(img.convert('L'))
    tgtimg = Image.new(mode = 'L', size = (pow2le(max([i.size[0] for i in srcimgs])), pow2le(sum([i.size[1] for i in srcimgs]))), color = 255)
    tgt_y = 0
    for s in srcimgs:
      # Copy the source image into a blank part of the target image
      tgtimg.paste(s, (0, tgt_y))
      tgt_y += s.size[1]

      # Divide up the source image into glyphs
      size_desc = lxml.etree.SubElement(desc_doc, "sizedesc", height=str(s.size[1]))
      x = 0
      for character in FONT_GLYPHS:
        # Advance until we see a non-blank row
        char_start = None
        while char_start is None:
          for y in xrange(0, s.size[1]):
            if s.getpixel((x, y)) < 250:
              char_start = x
              break
          x += 1
          if x == s.size[0]:
            raise RuntimeError("Size %u, Ran out of horizontal image space looking for start of char %s" % (s.size[1], character))

        # Now advance further until we see a blank row
        char_end = None
        while char_end is None:
          blank = True
          for y in xrange(0, s.size[1]):
            if s.getpixel((x, y)) < 250:
              blank = False
              break
          if blank:
            char_end = x-1
            break
          x += 1
          if x == s.size[0]:
            raise RuntimeError("Size %u, Ran out of horizontal image space looking for end of char %s" % (s.size[1], character))

        # Add this character
        glyph = lxml.etree.SubElement(size_desc, "glyph", character = character, offset = str(char_start), width = str(char_end - char_start + 1))
        #print "Size %u, Char %s, Offset %u, Width %u" % (s.size[1], character, char_start, char_end - char_start)

    font_desc_schema.assertValid(desc_doc)

    sym_name = "FONTDATA_" + f.upper()
    guard_name = "ORBIT_RIBBON_" + sym_name + "_H"
    char_array = img_to_png_char_array(tgtimg)

    header_fh = open("cpp/autofont/" + f + ".h", "w")
    header_fh.write(preamble_pattern % (f + ".h", "Automatically generated header for font %s" % f))
    header_fh.write("#ifndef %s\n" % guard_name)
    header_fh.write("#define %s\n" % guard_name)
    header_fh.write("\n")
    header_fh.write("const unsigned int %s_LEN = %u;\n" % (sym_name, len(char_array)))
    header_fh.write("extern const unsigned char %s[%u];\n" % (sym_name, len(char_array)))
    header_fh.write("extern const char* %s;\n" % (sym_name + "_DESC"))
    header_fh.write("\n")
    header_fh.write("#endif\n")
    header_fh.close()

    impl_fh = open("cpp/autofont/" + f + ".cpp", "w")
    impl_fh.write(preamble_pattern % (f + ".h", "Automatically generated data file for font %s" % f))
    impl_fh.write("#include \"%s.h\"\n" % f)
    impl_fh.write("\n")
    write_c_string(impl_fh, sym_name + "_DESC", lxml.etree.tostring(desc_doc, xml_declaration=True).split("\n"))
    impl_fh.write("\n")
    impl_fh.write("const unsigned char %s[%u] = {%s};\n" % (sym_name, len(char_array), ",".join(char_array)))
    impl_fh.close()


def font_emitter(target, source, env):
  fontnames = []
  for s in source:
    f = os.path.basename(os.path.dirname(str(s)))
    if f not in fontnames:
      fontnames.append(f)

  targets = []
  for f in fontnames:
    targets.append("cpp/autofont/%s.h" % f)
    targets.append("cpp/autofont/%s.cpp" % f)
  return (targets, source)


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

  if env.Execute(env.Action([["xsdcxx"] + args])):
    raise RuntimeError("build_xsd: xsdcxx call failed")
  for t in tgt_list:
    if env.Execute(env.Action(lambda target, source, env: file_sub(t, "long long", "long"), "%s updated to remove long long type" % (str(t)))):
      raise RuntimeError("build_xsd: long long removal failed")


def xsd_emitter(target, source, env):
  added = []
  target_strs = [str(t) for t in target]
  for t in target_strs:
    if t.endswith(".cpp"):
      header = t[:-4] + ".h"
      if header not in target_strs:
        added.append(header)
  return (target + added, source)


def build_verinfo(source, target, env):
  guard_name = "ORBIT_RIBBON_AUTOINFO_RELEASE_H"
  
  header_fh = open(os.path.join("cpp", "autoinfo", "version.h"), "w")
  header_fh.write(preamble_pattern % ("version.h", "Automatically generated header for Orbit Ribbon version/release info"))
  header_fh.write("#ifndef %s\n" % guard_name)
  header_fh.write("#define %s\n" % guard_name)
  header_fh.write("\n")
  header_fh.write("extern const char* const APP_VERSION;\n")
  header_fh.write("extern const char* const BUILD_DATE;\n")
  header_fh.write("extern const char* const COMMIT_DATE;\n")
  header_fh.write("extern const char* const COMMIT_HASH;\n")
  header_fh.write("\n")
  header_fh.write("#endif\n")
  header_fh.close()
  
  commit_hash_result = commands.getstatusoutput('git log -1 --format="format:%h"')
  commit_ts_result = commands.getstatusoutput('git log -1 --format="format:%at"')
  commit_date = datetime.datetime.fromtimestamp(int(commit_ts_result[1]) if commit_ts_result[0] == 0 else 0)
  build_date = datetime.datetime.now()
  date_format = "%a %b %d %Y %H:%M:%S"

  impl_fh = open(os.path.join("cpp", "autoinfo", "version.cpp"), "w")
  impl_fh.write(preamble_pattern % ("version.cpp", "Automatically generated source file for Orbit Ribbon version/release info"))
  impl_fh.write("#include \"version.h\"\n")
  impl_fh.write("\n")
  impl_fh.write("const char* const APP_VERSION = \"%s\";\n" % ORBIT_RIBBON_VERSION)
  impl_fh.write("const char* const BUILD_DATE = \"%s\";\n" % build_date.strftime(date_format))
  impl_fh.write("const char* const COMMIT_DATE = \"%s\";\n" % (commit_date.strftime(date_format) if commit_ts_result[0] == 0 else ""))
  impl_fh.write("const char* const COMMIT_HASH = \"%s\";\n" % (commit_hash_result[1] if commit_hash_result[0] == 0 else ""))
  impl_fh.close()

def verinfo_emitter(target, source, env):
  return ((os.path.join("cpp", "autoinfo", "version.h"), os.path.join("cpp", "autoinfo", "version.cpp")), ())

env.VariantDir('buildtmp', 'cpp', duplicate=0)

env['BUILDERS']['XSDTree'] = env.Builder(
  action = Action(lambda source, target, env: build_xsd('cxx-tree', source, target, env), "C++/Tree for XML schemas: $SOURCES"),
  suffix = {'.xsd' : '.cpp'},
  emitter = xsd_emitter
)
env['BUILDERS']['XSDParser'] = env.Builder(
  action = Action(lambda source, target, env: build_xsd('cxx-parser', source, target, env), "C++/Parser for XML schemas: $SOURCES"),
  suffix = {'.xsd' : '-pskel.cpp'},
  emitter = xsd_emitter
)
env['BUILDERS']['Capsulate'] = env.Builder(
  action = Action(build_capsulate, "Encapsulating to headers: $SOURCES"),
  suffix = {'.xsd' : '.h', '.xml' : '.h'}
)
env['BUILDERS']['CompileFonts'] = env.Builder(
  action = Action(build_font, "Generating font data from: $SOURCES"),
  emitter = font_emitter
)
env['BUILDERS']['VersionInfo'] = env.Builder(
  action = Action(build_verinfo, "Writing version info"),
  emitter = verinfo_emitter
)

tree_xsds = ['orepkgdesc', 'save', 'fontdesc']
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
fonts_built = env.CompileFonts(
  Glob('images/fonts/latinmodern/*.png', strings = True)
)
verinfo_built = env.VersionInfo()
AlwaysBuild(verinfo_built)

env.Program(
  'orbit-ribbon',
  [b for b in (tree_built + parser_built + fonts_built + verinfo_built) if str(b).endswith(".cpp")] + Glob('buildtmp/*.cpp') + Glob('cpp/minizip/*.c'),
  LIBS=['ode', 'SDL', 'SDL_image', 'z', 'boost_system', 'boost_filesystem', 'boost_program_options', 'boost_iostreams', 'xerces-c', 'glew32', 'opengl32', 'glu32', 'm', 'user32', 'gdi32', 'winmm'],
  CCFLAGS=CCFLAGS,
  LINKFLAGS='-static'
  #LIBS=['GL', 'GLU', 'GLEW', 'ode', 'SDL', 'SDL_image', 'zzip', 'boost_filesystem', 'boost_program_options', 'boost_iostreams', 'xerces-c'],
)
