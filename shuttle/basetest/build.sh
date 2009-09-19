#!/bin/sh
gcc test.c -o test -lGL -lGLU `sdl-config --cflags --libs`
