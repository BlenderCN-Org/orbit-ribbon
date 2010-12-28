/*
ore.cpp: Implementation of resource management classes.
These classes are responsible for loading and interpreting game resources from ORE files.

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

#include <algorithm>
#include <vector>
#include <string>
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

// Functions to allow OreFileHandle to work with SDL_RWops
int sdl_rwops_seek(
  SDL_RWops* context __attribute__ ((unused)),
  int offset __attribute__ ((unused)),
  int whence __attribute__ ((unused))
) {
  /*
  zzip_seek((ZZIP_FILE*)context->hidden.unknown.data1, offset, whence);
  return zzip_tell((ZZIP_FILE*)context->hidden.unknown.data1);
  */
  throw OreException("Attempted to seek ORE filehandle via SDL RWops");
}

int sdl_rwops_read(SDL_RWops* context, void* ptr, int size, int maxnum) {
  int read_amt = unzReadCurrentFile((unzFile)context->hidden.unknown.data1, ptr, size*maxnum);
  if (read_amt < 0) {
    throw OreException("Error while reading file from ORE in sdl_rwops_read");
  }
  return read_amt;
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
  
  int read_amt = unzReadCurrentFile(_ofhp->_pkg.uf, &_in_buf, ORE_CHUNK_SIZE);
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
  sb.set_ofh(*this);
  exceptions(std::istream::badbit | std::istream::failbit); // Have istream throw an exception if it has any problems
  
  if (_pkg.locked) {
    throw OreException("Unable to request file '" + name + "' from ORE package, as another file handle is already open");
  }
  _pkg.locked = true;
  int err = unzLocateFile(_pkg.uf, name.c_str(), 1);
  if (err != UNZ_OK) {
    throw OreException("Unable to locate file '" + name + "' from ORE package");
  }
  err = unzOpenCurrentFile(_pkg.uf);
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
  ret.hidden.unknown.data1 = (void*)(_pkg.uf);
  return ret;
}

OreFileHandle::~OreFileHandle() {
  unzCloseCurrentFile(_pkg.uf);
  _pkg.locked = false;
}

OrePackage::OrePackage(const boost::filesystem::path& p) : path(p), locked(false) {
  uf = unzOpen64(p.string().c_str());
  if (!uf) {
    throw OreException("Unable to open ORE package '" + p.string() + "' with minizip library");
  }
  
  try {
    {
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
      // Here fh goes out of scope and unlocks the package to open another file 
    }
    
    OreFileHandle pdesc_fh(*this, "ore-desc");
    pkg_desc = boost::shared_ptr<ORE1::PkgDescType>(ORE1::pkgDesc(pdesc_fh, "ore-desc", xsd::cxx::tree::flags::dont_validate));
  } catch (const std::exception& e) {
    unzClose(uf);
    throw OreException(std::string("Error while opening ORE package : ") + e.what());
  }
  
  // TODO Implement looking in other OrePackages for files not found in this one ("base packages")
}

OrePackage::~OrePackage() {
  unzClose(uf);
}

boost::shared_ptr<OreFileHandle> OrePackage::get_fh(const std::string& name) {
  // TODO Implement looking in other OrePackages for files not found in this one ("base packages")
  return boost::shared_ptr<OreFileHandle>(new OreFileHandle(*this, name));
}
