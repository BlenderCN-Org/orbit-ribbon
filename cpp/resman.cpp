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

#include <algorithm>
#include <vector>
#include <string>
#include <zzip/zzip.h>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "constants.h"
#include "debug.h"
#include "except.h"
#include "resman.h"
#include "autoxsd/orepkgdesc.h"

std::vector<boost::filesystem::path> loaded_ore_package_paths;
boost::shared_ptr<OrePackage> top_ore_package;

OreFileHandle::OreFileHandle(OrePackage& pkg, const std::string& name, bool dont_own_origin) {
	// This is a stupid hack to allow OreFileHandles to be used and safely destroyed within the constructor of OrePackage
	// Without it in that scenario, when the origin smart pointer is destroyed, it attempts to destruct an incomplete OrePackage
	if (!dont_own_origin) {
		origin.reset(&pkg);
	}
	
	fp = zzip_file_open(pkg.zzip_h, name.c_str(), 0);
	if (!fp) {
		throw OreException("Unable to open file " + name + " from ORE package");
	}
}

unsigned int OreFileHandle::read(char* buf, unsigned int len) {
	return zzip_file_read(fp, buf, len);
}

void OreFileHandle::rewind() {
	zzip_seek(fp, 0, SEEK_SET);
}

void OreFileHandle::append_to_string(std::string& tgt) {
	char buf[ORE_CHUNK_SIZE];
	unsigned int len;
	while ((len = read(buf, ORE_CHUNK_SIZE))) {
		tgt.append(buf, len);
	}
}

OreFileHandle::~OreFileHandle() {
	zzip_file_close(fp);
}

OrePackage::OrePackage(const boost::filesystem::path& p) : path(p) {
	BOOST_FOREACH(boost::filesystem::path loaded_path, loaded_ore_package_paths) {
		if (boost::filesystem::equivalent(p, loaded_path)) {
			throw OreException("Attempted to double-load ORE package " + boost::filesystem::complete(p).string());
		}
	}
	
	zzip_h = zzip_dir_open(p.string().c_str(), 0);
	if (!zzip_h) {
		throw OreException("Unable to open ORE package " + p.string() + " with libzzip");
	}
	
	try {
		OreFileHandle fh(*this, "ore-version", true);
		char buf[16];
		unsigned int len = fh.read(buf, 16);
		if (len) {
			std::string ver(buf, len);
			if (ver != "1") {
				throw OreException("ORE package has unrecognized version '" + ver + "'. Update your copy of Orbit Ribbon!");
			}
		} else {
			throw OreException("Unable to read from ore-version file within ORE package");
		}
		
		OreFileHandle pdesc_fh(*this, "ore-desc", true);
		pkg_desc = boost::shared_ptr<ORE1::PkgDescType>(ORE1::pkgDesc(pdesc_fh, "ore-desc"));
		
		loaded_ore_package_paths.push_back(p);
	} catch (const std::exception& e) {
		zzip_dir_close(zzip_h);
		throw e;
	}
	
	// TODO Implement looking in other OrePackages for files not found in this one ("base packages")
}

OrePackage::~OrePackage() {
	zzip_dir_close(zzip_h);
	loaded_ore_package_paths.erase(std::find(loaded_ore_package_paths.begin(), loaded_ore_package_paths.end(), path));
}

boost::shared_ptr<OreFileHandle> OrePackage::get_fh(const std::string& name) {
	// TODO Implement looking in other OrePackages for files not found in this one ("base packages")
	return boost::shared_ptr<OreFileHandle>(new OreFileHandle(*this, name, false));
}

boost::shared_ptr<OreFileHandle> ResMan::get_fh(const std::string& name) {
	if (!top_ore_package) {
		throw OreException("Attempted to get filehandle from uninitialized ResMan");
	}
	
	return top_ore_package->get_fh(name);
}

std::vector<boost::filesystem::path> ore_pkg_std_locations; // Standard directories to look for ORE files in

void ResMan::_init(const std::string& top_ore_package_name) {
	if (ore_pkg_std_locations.size() == 0) {
		ore_pkg_std_locations.push_back(boost::filesystem::initial_path() / "orefiles");
		// TODO Add more locations here, depending on OS and installation location
	}
	
	// Unload the current package, if any
	top_ore_package.reset();
	
	// TODO Check if top_ore_package_name is a fully qualified path instead of just a filename
	bool loaded = false;
	BOOST_FOREACH(boost::filesystem::path loc, ore_pkg_std_locations) {
		boost::filesystem::path p = loc / top_ore_package_name;
		try {
			top_ore_package = boost::shared_ptr<OrePackage>(new OrePackage(p));
			loaded = true;
			Debug::status_msg("Loaded ORE package '" + top_ore_package_name + "' from location " + p.string());
			break;
		} catch (const OreException& e) {
			Debug::status_msg("Unable to load ORE package '" + top_ore_package_name + "' from location " + p.string() + " : " + e.get_msg());
			continue;
		}
	}
	
	if (!loaded) {
		throw OreException("Unable to load ORE package '" + top_ore_package_name + "'!");
	}
}