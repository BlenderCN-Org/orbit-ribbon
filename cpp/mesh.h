/*
mesh.h: Header for mesh and animation classes.
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

#ifndef ORBIT_RIBBON_MESH_H
#define ORBIT_RIBBON_MESH_H

#include <string>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include "gloo.h"

class MeshAnimation;
class _AnimationParser;
class _Mesh;
class _MeshParser;

class MeshAnimation : boost::noncopyable {
	private:
		std::vector<boost::shared_ptr<GLOOBufferedMesh> > frames;
		std::string name;
		
		MeshAnimation() {}
		
		friend class _MeshAnimationParser;
	
	public:
		static boost::shared_ptr<MeshAnimation> create(const std::string& name);
		
		const std::string& get_name() { return name; }
};

#endif
