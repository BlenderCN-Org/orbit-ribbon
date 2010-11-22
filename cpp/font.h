/*
font.h: Header for Orbit Ribbon's simple font system.

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

#ifndef ORBIT_RIBBON_FONT_H
#define ORBIT_RIBBON_FONT_H

#include <boost/shared_ptr.hpp>
#include <map>

#include "geometry.h"
#include "gloo.h"

// Special glyph values, to do something other than display the given offset image
const short SPECIAL_GLYPH_SPACE = -1; // Advance the cursor by average glyph width
const short SPECIAL_GLYPH_DOUBLE_QUOTE = -2; // Display two single-quote characters

class Font {
  private:
    boost::shared_ptr<GLOOTexture> _tex;

    // Key is max glyph height, value maps input char to offset and width, or to one of the SPECIAL_GLYPH_* values and 0.
    std::map<unsigned char, std::map<char, std::pair<short, unsigned char> > > _glyph_data;
  
  public:
    Font(unsigned char* img_data, unsigned int img_data_len, unsigned char* glyph_desc);
    
    float get_width(float height, const std::string& str);
    void draw(const Point& upper_left, float height, const std::string& str);
};

#endif
