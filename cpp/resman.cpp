/*
resman.cpp: Implementation of resource management classes.
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

#include <vector>
#include <string>
#include <zzip/zzip.h>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

#include "except.h"
#include "resman.h"

std::vector<boost::filesystem::path> loaded_ore_package_paths;
boost::shared_ptr<OrePackage> top_ore_package;
unsigned int opened_ore_file_handles = 0;

OreFileHandle::OreFileHandle(OrePackage& pkg, const std::string& name) {
	origin.reset(&pkg);
	++opened_ore_file_handles;
}

OreFileHandle::~OreFileHandle() {
	--opened_ore_file_handles;
}

OrePackage::OrePackage(const boost::filesystem::path& p) {
	BOOST_FOREACH( boost::filesystem::path loaded_path, loaded_ore_package_paths) {
		if (boost::filesystem::equivalent(p, loaded_path)) {
			throw GameException("Attempted to double-load ORE package " + boost::filesystem::complete(p).string());
		}
	}
}

OrePackage::~OrePackage() {
}

boost::shared_ptr<OreFileHandle> OrePackage::get_fh(const std::string& name) {
}

boost::shared_ptr<OreFileHandle> ResMan::get_fh(const std::string& name) {
	if (!top_ore_package) {
		throw GameException("Attempted to get filehandle from uninitialized ResMan");
	}
	
	return top_ore_package->get_fh(name);
}

void ResMan::_init(const boost::filesystem::path& top_ore_package_path) {
	if (opened_ore_file_handles != 0) {
		throw GameException("Attempted to open a top ORE package while other OreFileHandles still open");
	}
}