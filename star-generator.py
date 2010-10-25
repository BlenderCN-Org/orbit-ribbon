#!/usr/bin/python

import os, sys, re, Image, random

MIN_BRIGHTNESS = 10
MAX_BRIGHTNESS = 255
STARS = 25000

mask_fn = sys.argv[1]
mask = Image.open(mask_fn) 
img = Image.new(mode = 'L', size = mask.size, color = 0)

random.seed()
for i in xrange(STARS):
  pos = (random.randrange(0, mask.size[0]), random.randrange(0, mask.size[1]))
  if mask.getpixel(pos) < random.randrange(0, 220):
    img.putpixel(pos, random.randint(MIN_BRIGHTNESS, MAX_BRIGHTNESS))

ext_pat = re.compile(r"^(.+)\.(.+?)$")
ext_match = ext_pat.match(mask_fn)
img.save("%s-out.%s" % (ext_match.group(1), ext_match.group(2)))
