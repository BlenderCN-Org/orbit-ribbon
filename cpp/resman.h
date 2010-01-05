/*
resman.h: Header for resource management classes.
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

#ifndef ORBIT_RIBBON_RESMAN_H
#define ORBIT_RIBBON_RESMAN_H

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <istream>
#include <string>
#include <vector>
#include <zzip/zzip.h>

#include "autoxsd/orepkgdesc.h"
#include "except.h"
#include "constants.h"

class OreException : public GameException {
	public:
		OreException(const std::string& msg) : GameException(msg) {}
		virtual ~OreException() throw() {}
};

class OrePackage;
class OreFileHandle;

// Allows you to read information from a particular file in an ORE package
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
		
		ZZIP_FILE* fp;
		OFHStreamBuf sb;
		
		OreFileHandle(const OrePackage& pkg, const std::string& name);
		
		friend class OFHStreamBuf;
		friend class OrePackage;
	
	public:
		virtual ~OreFileHandle();
};

// Represents an opened ORE package
class OrePackage : boost::noncopyable {
	private:
		boost::filesystem::path path;
		std::vector<boost::shared_ptr<OrePackage> > base_pkgs;
		ZZIP_DIR* zzip_h;
		boost::shared_ptr<ORE1::PkgDescType> pkg_desc;
		
		OrePackage(const boost::filesystem::path& p);
		
		friend class OreFileHandle;
		friend class ResMan; // Only ResMan can construct OrePackages, since all OrePackages must be owned by shared_ptrs
	public:
		~OrePackage();
		
		boost::shared_ptr<OreFileHandle> get_fh(const std::string& name) const;
		const ORE1::PkgDescType& get_pkg_desc() const { return *pkg_desc; }
};

class App;

// Resource management
class ResMan {
	public:
		static const OrePackage& pkg();
		
	private:
		// Initializes the resource manager with the given ORE package
		// Supply a full path or else a filename to look for in the standard locations
		// Can be called again in order to switch to a different top ORE package
		// But before doing so, make sure all OreFileHandles are closed!
		static void _init(const std::string& top_ore_package_name);
		
		friend class App;
};

#endif
