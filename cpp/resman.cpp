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