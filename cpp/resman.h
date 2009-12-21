#ifndef ORBIT_RIBBON_RESMAN_H
#define ORBIT_RIBBON_RESMAN_H

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <string>
#include <vector>
#include <zzip/zzip.h>

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