/*
font.h: Header for Orbit Ribbon's simple font system.

Copyright 2011 David Simon <david.mike.simon@gmail.com>

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

class Point;

class Font {
  private:
    // Key is max glyph height, value maps input char to offset and width. Offset is -1 for no character (i.e. space)
    std::map<unsigned char, std::map<char, std::pair<short, unsigned char> > > _glyph_data;

    std::pair<unsigned char, short> get_glyph_height_and_y_offset(float height);
  
  public:
    Font(const unsigned char* img_data, unsigned int img_data_len, const char* font_desc);
    
    float get_width(float height, const std::string& str);
    void draw(const Point& upper_left, float height, const std::string& str);
};

#endif
