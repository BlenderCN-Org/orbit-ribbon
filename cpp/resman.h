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
#include <string>
#include <vector>
#include <zzip/zzip.h>

#include "autoxsd/orepkgdesc.h"
#include "except.h"

class OreException : public GameException {
	public:
		OreException(const std::string& msg) : GameException(msg) {}
};

class OrePackage;
class OreFileHandle;

class OreFileHandle : boost::noncopyable {
	private:
		ZZIP_FILE* fp;
		boost::shared_ptr<OrePackage> origin;
		
		OreFileHandle(OrePackage& pkg, const std::string& name, bool dont_own_origin);
		
		friend class OrePackage;
	
	public:
		unsigned int read(char* buf, unsigned int len);
		void rewind();
		
		~OreFileHandle();
};

class OrePackage : boost::noncopyable {
	private:
		boost::filesystem::path path;
		std::vector<boost::shared_ptr<OrePackage> > base_pkgs;
		ZZIP_DIR* zzip_h;
		
		OrePackage(const boost::filesystem::path& p);
		
		friend class OreFileHandle;
		friend class ResMan; // Only ResMan can construct OrePackages, since all OrePackages must be owned by shared_ptrs
	public:
		
		~OrePackage();
		
		boost::shared_ptr<OreFileHandle> get_fh(const std::string& name);
};

class App;

// Resource management
class ResMan {
	public:
		// Returns an OreFileHandle from the current ORE package
		static boost::shared_ptr<OreFileHandle> get_fh(const std::string& name);
		
	private:
		// Initializes the resource manager with the given ORE package
		// Supply a full path or else a filename to look for in the standard locations
		// Can be called again in order to switch to a different top ORE package
		// But before doing so, make sure all OreFileHandles are closed!
		static void _init(const std::string& top_ore_package_name);
		
		friend class App;
};

#endif
