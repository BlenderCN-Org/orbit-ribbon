/*
ore.cpp: Implementation of resource management classes.
These classes are responsible for loading and interpreting game resources from ORE files.

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

#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <SDL/SDL.h>

#include "constants.h"
#include "debug.h"
#include "except.h"
#include "ore.h"
#include "autoxsd/orepkgdesc.h"
#include "autoxsd/orepkgdesc-pimpl.h"

// Functions to allow OreFileHandle to work with SDL_RWops
int sdl_rwops_read(SDL_RWops* context, void* ptr, int size, int maxnum) {
  int read_amt = unzReadCurrentFile((unzFile)context->hidden.unknown.data1, ptr, size*maxnum);
  if (read_amt < 0) {
    throw OreException("Error while reading file from ORE in sdl_rwops_read");
  }
  return read_amt;
}

int sdl_rwops_seek(SDL_RWops* context, int offset, int whence) {
  if (whence == RW_SEEK_CUR) {
    if (offset < 0) {
      throw OreException("Cannot seek backwards");
    } else if (offset > 0) {
      // Read offset bytes from the file into a garbage buffer
      const int BUF_SIZE = 512;
      static char buf[BUF_SIZE];
      int remaining = offset;
      while (remaining > 0) {
        int readlen = remaining;
        if (readlen > BUF_SIZE) {
          readlen = BUF_SIZE;
        }
        sdl_rwops_read(context, (void*)buf, readlen, 1);
        remaining -= readlen;
      }
    }
  } else {
    throw OreException("Invalid whence value");
  }
  return unztell((unzFile)context->hidden.unknown.data1);
}

int sdl_rwops_write(
  SDL_RWops* context __attribute__ ((unused)),
  const void* ptr __attribute__ ((unused)),
  int size __attribute__ ((unused)),
  int num __attribute__ ((unused)))
{
  throw OreException("Attempted to write to ORE filehandle via SDL RWops");
}

int sdl_rwops_close(SDL_RWops* context __attribute__ ((unused))) {
  // Do nothing; de-initialization will occur in OreFileHandle dtor
  return 0;
}

void OreFileHandle::OFHStreamBuf::_reset_sptrs(unsigned int len) {
  setg(&(_in_buf[0]), &(_in_buf[0]), &(_in_buf[0]) + len);
}

OreFileHandle::OFHStreamBuf::OFHStreamBuf() {
  _reset_sptrs(0);
}

void OreFileHandle::OFHStreamBuf::set_ofh(OreFileHandle& ofh) {
  _ofhp = &ofh;
}

int OreFileHandle::OFHStreamBuf::underflow() {
  if (gptr() < egptr()) {
    return *gptr();
  }
  
  if (_ofhp == 0) {
    throw OreException("Attempted to read from uninitialized OFHStreamBuf");
  }
  
  int read_amt = unzReadCurrentFile(_ofhp->uf, &_in_buf, ORE_CHUNK_SIZE);
  if (read_amt < 0) {
    throw OreException("Error while reading file from ORE in underflow");
  }
  _reset_sptrs(read_amt);
  if (read_amt == 0) {
    return std::char_traits<char>::eof();
  } else {
    return sgetc();
  }
}

OreFileHandle::OreFileHandle(OrePackage& pkg, const std::string& name) :
  std::istream(&sb),
  _pkg(pkg)
{
  uf = unzOpen64(_pkg.path.string().c_str());
  if (!uf) {
    throw OreException("Unable to open ORE package '" + _pkg.path.string() + "' with minizip library");
  }

  sb.set_ofh(*this);
  exceptions(std::istream::badbit | std::istream::failbit); // Have istream throw an exception if it has any problems
  
  int err = unzLocateFile(uf, name.c_str(), 1);
  if (err != UNZ_OK) {
    throw OreException("Unable to locate file '" + name + "' from ORE package");
  }
  err = unzOpenCurrentFile(uf);
  if (err != UNZ_OK) {
    throw OreException("Error opening file '" + name + "' from ORE package");
  }
}

SDL_RWops OreFileHandle::get_sdl_rwops() {
  SDL_RWops ret;
  ret.seek = &sdl_rwops_seek;
  ret.read = &sdl_rwops_read;
  ret.write = &sdl_rwops_write;
  ret.close = &sdl_rwops_close;
  ret.hidden.unknown.data1 = (void*)(uf);
  return ret;
}

OreFileHandle::~OreFileHandle() {
  unzCloseCurrentFile(uf);
  unzClose(uf);
}

OrePackage::OrePackage(const boost::filesystem::path& p) : path(p) {
  try {
    OreFileHandle fh(*this, "ore-version");
    int ver;
    fh >> ver;
    if (ver != 1) {
      throw OreException(
        std::string("Unrecognized ORE package version '") +
        boost::lexical_cast<std::string>(ver) +
        "'. Install a newer version of Orbit Ribbon!"
      );
    }
    
    OreFileHandle pdesc_fh(*this, "ore-desc");
    ORE1::pkgDesc_paggr pkgDesc_p;
    xml_schema::document_pimpl doc_p(pkgDesc_p.root_parser(), pkgDesc_p.root_namespace(), pkgDesc_p.root_name(), true);
    pkgDesc_p.pre();
    doc_p.parse(pdesc_fh);
    pkg_desc = boost::shared_ptr<ORE1::PkgDescType>(pkgDesc_p.post());
  } catch (const xml_schema::parser_exception& e) {
    throw OreException(std::string("Parsing problem while opening ORE package : ") + e.text());
  } catch (const std::exception& e) {
    throw OreException(std::string("Error while opening ORE package : ") + e.what());
  }
  
  // TODO Implement looking in other OrePackages for files not found in this one ("base packages")
}

boost::shared_ptr<OreFileHandle> OrePackage::get_fh(const std::string& name) {
  // TODO Implement looking in other OrePackages for files not found in this one ("base packages")
  return boost::shared_ptr<OreFileHandle>(new OreFileHandle(*this, name));
}
