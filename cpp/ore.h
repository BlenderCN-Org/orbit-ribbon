/*
ore.h: Header for resource management classes.
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

#ifndef ORBIT_RIBBON_RESMAN_H
#define ORBIT_RIBBON_RESMAN_H

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>
#include <istream>
#include <string>
#include <vector>
#include <SDL/SDL.h>

#include "autoxsd/orepkgdesc.h"
#include "minizip/unzip.h"
#include "except.h"
#include "constants.h"

// How many bytes to load from ORE files in each chunk
const int ORE_CHUNK_SIZE = 4096;

class OreException : public GameException {
  public:
    OreException(const std::string& msg) : GameException(msg) {}
    virtual ~OreException() throw() {}
};

class OrePackage;
class OreFileHandle;
class OreFileData;

// Allows you to stream information from a particular file in an ORE package
class OreFileHandle : boost::noncopyable, public std::istream {
  private:
    class OFHStreamBuf : public std::streambuf {
      private:
        char _in_buf[ORE_CHUNK_SIZE];
        OreFileHandle* _ofhp;
        
        void _reset_sptrs(unsigned int len);
      
      public:
        OFHStreamBuf();
        
        void set_ofh(OreFileHandle& ofh);
      
      protected:
        int underflow();
    };
    
    unzFile uf;
    OFHStreamBuf sb;
    OrePackage& _pkg;
    
    OreFileHandle(OrePackage& pkg, const std::string& name);
    
    friend class OFHStreamBuf;
    friend class OrePackage;
  
  public:
    unsigned long uncompressed_size();

    virtual ~OreFileHandle();
};

// Loads all the data from a particular file in an ORE package into memory
class OreFileData : boost::noncopyable {
  private:
    char* _data;
    int _size;
    SDL_RWops* _rwops;

    OreFileData(OreFileHandle& fh);

    friend class OrePackage;

  public:
    virtual ~OreFileData();
    SDL_RWops* get_const_sdl_rwops();
};


// Represents an opened ORE package
class OrePackage : boost::noncopyable {
  private:
    boost::filesystem::path path;
    std::vector<boost::shared_ptr<OrePackage> > base_pkgs;
    boost::shared_ptr<ORE1::PkgDescType> pkg_desc;
    
    friend class OreFileHandle;
  public:
    OrePackage(const boost::filesystem::path& p);
    
    boost::shared_ptr<OreFileHandle> get_fh(const std::string& name);
    boost::shared_ptr<OreFileData> get_data(const std::string& name);
    const ORE1::PkgDescType& get_pkg_desc() const { return *pkg_desc; }
};

#endif
