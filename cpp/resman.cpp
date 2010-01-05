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
#include <memory>
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
	
	unsigned int read_amt = zzip_file_read(_ofhp->fp, &_in_buf, ORE_CHUNK_SIZE);
	_reset_sptrs(read_amt);
	if (read_amt == 0) {
		return std::char_traits<char>::eof();
	} else {
		return sgetc();
	}
}

OreFileHandle::OreFileHandle(const OrePackage& pkg, const std::string& name) :
	std::istream(&sb)
{
	sb.set_ofh(*this);
	exceptions(std::istream::badbit | std::istream::failbit); // Have istream throw an exception if it has any problems
	
	fp = zzip_file_open(pkg.zzip_h, name.c_str(), 0);
	if (!fp) {
		throw OreException("Unable to open file " + name + " from ORE package");
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
		OreFileHandle fh(*this, "ore-version");
		int ver;
		fh >> ver;
		if (ver != 1) {
			throw OreException(
				std::string("Unrecognized ORE package version '") +
				boost::lexical_cast<std::string>(ver) +
				"'. Update your copy of Orbit Ribbon!"
			);
		}
		
		OreFileHandle pdesc_fh(*this, "ore-desc");
		pkg_desc = boost::shared_ptr<ORE1::PkgDescType>(ORE1::pkgDesc(pdesc_fh, "ore-desc", xsd::cxx::tree::flags::dont_validate));
		
		loaded_ore_package_paths.push_back(p);
	} catch (const std::exception& e) {
		zzip_dir_close(zzip_h);
		throw OreException(std::string("Error while opening ORE package : ") + e.what());
	}
	
	// TODO Implement looking in other OrePackages for files not found in this one ("base packages")
}

OrePackage::~OrePackage() {
	zzip_dir_close(zzip_h);
	loaded_ore_package_paths.erase(std::find(loaded_ore_package_paths.begin(), loaded_ore_package_paths.end(), path));
}

boost::shared_ptr<OreFileHandle> OrePackage::get_fh(const std::string& name) const {
	// TODO Implement looking in other OrePackages for files not found in this one ("base packages")
	return boost::shared_ptr<OreFileHandle>(new OreFileHandle(*this, name));
}

const OrePackage& ResMan::pkg() {
	if (!top_ore_package) {
		throw OreException("Attempted to get pkg from uninitialized ResMan");
	}
	
	return *top_ore_package;
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