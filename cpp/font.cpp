/*
font.cpp: Implementation for Orbit Ribbon's simple font system.

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

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/array.hpp>
#include <cstring>
#include <cmath>
#include <memory>
#include <SDL/SDL.h>

#include "except.h"
#include "font.h"
#include "gloo.h"

#include "autoxsd/fontdesc.h"
#include "autoxsd/fontdesc-pimpl.h"

const unsigned int FONT_MAX_STR_LENGTH = 512;

std::pair<unsigned char, short> Font::get_glyph_height_and_y_offset(float height) {
  std::pair<unsigned char, short> r(0, 0);
  for (std::map<unsigned char, std::map<char, std::pair<short, unsigned char> > >::iterator i = _glyph_data.begin(); i != _glyph_data.end(); ++i) {
    if (i->first > height) {
      break;
    }
    r.first = i->first;
    r.second += i->first;
  }
  r.second -= r.first;
  return r;
}

Font::Font(const unsigned char* img_data, unsigned int img_data_len, const char* font_desc_str) {
  try {
    boost::iostreams::array_source font_desc_src(font_desc_str, std::strlen(font_desc_str));
    boost::iostreams::stream<boost::iostreams::array_source> font_desc_stream(font_desc_src);
    ORFontDesc::fontdesc_paggr fontdesc_p;
    xml_schema::document_pimpl doc_p(fontdesc_p.root_parser(), fontdesc_p.root_namespace(), fontdesc_p.root_name());
    fontdesc_p.pre();
    doc_p.parse(font_desc_stream);
    std::auto_ptr<ORFontDesc::FontDescType> font_desc(fontdesc_p.post());
  
    for (ORFontDesc::FontDescType::sizedesc_const_iterator sd = font_desc->sizedesc().begin(); sd != font_desc->sizedesc().end(); ++sd) {
      std::map<char, std::pair<short, unsigned char> >& glyphmap = _glyph_data[sd->height()];
      for (ORFontDesc::SizeDescType::glyph_const_iterator gd = sd->glyph().begin(); gd != sd->glyph().end(); ++gd) {
        glyphmap[gd->character()[0]] = std::pair<short, unsigned char>(gd->offset(), gd->width());
      }
      glyphmap[' '] = std::pair<short, unsigned char>(-1, glyphmap['h'].second/2); // Make spaces half as wide as the letter 'h'
    }

    SDL_RWops* img_rw_ops(SDL_RWFromConstMem(img_data, img_data_len));
    _tex.reset(new GLOOTexture(*img_rw_ops, true));
    SDL_FreeRW(img_rw_ops);
  } catch (const xml_schema::parser_exception& e) {
    throw GameException(std::string("Parsing problem while reading font description : ") + e.text());
  } catch (const std::exception& e) {
    throw GameException(std::string("Error while loading font : ") + e.what());
  }
}

float Font::get_width(float height, const std::string& str) {
  unsigned char h = get_glyph_height_and_y_offset(height).first;
  std::map<char, std::pair<short, unsigned char> >& glyphmap = _glyph_data[h];

  float width = 0.0;
  unsigned int glyphs = 0;
  for (std::string::const_iterator c = str.begin(); c != str.end(); ++c) {
    std::map<char, std::pair<short, unsigned char> >::iterator g = glyphmap.find(*c);
    if (g != glyphmap.end()) {
      width += g->second.second + 1;
      if (g->second.first >= 0) {
        ++glyphs;
        if (glyphs == FONT_MAX_STR_LENGTH) {
          break;
        }
      }
    }
  }
  return width;
}

void Font::draw(const Point& upper_left, float height, const std::string& str) {
  std::pair<unsigned char, short> hy = get_glyph_height_and_y_offset(height);
  std::map<char, std::pair<short, unsigned char> >& glyphmap = _glyph_data[hy.first];

  static GLfloat points[FONT_MAX_STR_LENGTH*8];
  static GLfloat uv_points[FONT_MAX_STR_LENGTH*8];
  float x = 0.0;
  Size t = _tex->get_size();
  unsigned int glyphs = 0;
  for (std::string::const_iterator c = str.begin(); c != str.end(); ++c) {
    std::map<char, std::pair<short, unsigned char> >::iterator g = glyphmap.find(*c);
    if (g != glyphmap.end()) {
      if (g->second.first >= 0) {
        // 0 1
        points[glyphs*8 + 0] = x; points[glyphs*8 + 1] = hy.first;
        uv_points[glyphs*8 + 0] = float(g->second.first)/t.x; uv_points[glyphs*8 + 1] = float(hy.second + hy.first)/t.y;

        // 1 1
        points[glyphs*8 + 2] = x + g->second.second; points[glyphs*8 + 3] = hy.first;
        uv_points[glyphs*8 + 2] = float(g->second.first + g->second.second)/t.x; uv_points[glyphs*8 + 3] = float(hy.second + hy.first)/t.y;

        // 1 0
        points[glyphs*8 + 4] = x + g->second.second; points[glyphs*8 + 5] = 0.0;
        uv_points[glyphs*8 + 4] = float(g->second.first + g->second.second)/t.x; uv_points[glyphs*8 + 5] = float(hy.second)/t.y;

        // 0 0
        points[glyphs*8 + 6] = x; points[glyphs*8 + 7] = 0.0;
        uv_points[glyphs*8 + 6] = float(g->second.first)/t.x; uv_points[glyphs*8 + 7] = float(hy.second)/t.y;
        
        ++glyphs;
        if (glyphs == FONT_MAX_STR_LENGTH) {
          break;
        }
      }
      x += g->second.second + 1;
    }
  }

  GLOOPushedMatrix pm;
  _tex->bind();
  glTranslatef(upper_left.x, upper_left.y + std::floor((height - hy.first)/2 + height*0.12 + 0.5), 0);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
  glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
  glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDisableClientState(GL_NORMAL_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, points);
  glTexCoordPointer(2, GL_FLOAT, 0, uv_points);
  glDrawArrays(GL_QUADS, 0, glyphs*4);
  glPopClientAttrib();
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}
