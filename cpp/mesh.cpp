/*
mesh.cpp: Implementation for mesh and animation classes.
This module handles loading and displaying meshes, animations, and textures

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

#include <string>
#include <boost/shared_ptr.hpp>

#include "autoxsd/oreanim-pskel.h"
#include "cache.h"
#include "mesh.h"
#include "resman.h"

class MeshAnimationCache : public CacheBase<MeshAnimation> {
	boost::shared_ptr<MeshAnimation> generate(const std::string& id) {
	}
};

MeshAnimationCache mesh_animation_cache;

boost::shared_ptr<MeshAnimation> MeshAnimation::create(const std::string& name) {
	return mesh_animation_cache.get(name);
}