#!/usr/bin/python

from distutils.core import setup
from Pyrex.Distutils.extension import Extension
from Pyrex.Distutils import build_ext

setup(
	name = 'Shuttle Backend',
	ext_modules = [
		Extension("shuttle_backend", ["shuttle_backend.pyx"]),
	],
	cmdclass = {'build_ext': build_ext}
)
