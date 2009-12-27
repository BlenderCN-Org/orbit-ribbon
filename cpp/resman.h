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

class OrePackage;
class OreFileHandle;

class OreFileHandle : boost::noncopyable {
	private:
		boost::shared_ptr<OrePackage> origin;
		OreFileHandle(OrePackage& pkg, const std::string& name);
	
	public:
		~OreFileHandle();
};

class OrePackage : boost::noncopyable {
	private:
		std::vector<boost::shared_ptr<OrePackage> > base_pkgs;
		
		friend class OreFileHandle;
		
	public:
		OrePackage(const boost::filesystem::path& p);
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
		// Can be called again in order to switch to a different top ORE package
		// But before doing so, make sure all OreFileHandles are closed!
		static void _init(const boost::filesystem::path& top_ore_package_path);
		
		friend class App;
};

#endif
