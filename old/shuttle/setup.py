#!/usr/bin/python

from distutils.core import setup
from Pyrex.Distutils.extension import Extension
from Pyrex.Distutils import build_ext

setup(
	name = 'Shuttle Backend',
	ext_modules = [
		Extension(
			name = "shuttle_backend",
			sources = ["shuttle_backend.pyx"],
			include_dirs = ["/usr/include/SDL", "/usr/include/GL"],
			libraries = ["SDL", "GL", "GLU"],
		),
	],
	cmdclass = {'build_ext': build_ext}
)
