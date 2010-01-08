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

class _MeshAnimationParser : public ORE1::AnimationType_pskel {
	private:
		MeshAnimation* _anim_p;
	
	public:
		_MeshAnimationParser() : _anim_p(0) {}
		
		void pre() {
			_anim_p = new MeshAnimation();
		}
		
		void frame() {
		}
		
		void name(const std::string& n) {
			_anim_p->name = n;
		}
		
		MeshAnimation* post_AnimationType() {
			return _anim_p;
		}
};

class _MeshParser : public ORE1::MeshType_pskel {
};

class _FaceParser : public ORE1::FaceType_pskel {
};

class _VertexParser : public ORE1::VertexType_pskel {
};

// No need to make a parser for each subtype of FloatList/IntList, since they'd all do the same thing: append to the VBO

class _IntListParser : public ORE1::IntListType_pskel {
};

class _FloatListParser : public ORE1::FloatListType_pskel {
};

class MeshAnimationCache : public CacheBase<MeshAnimation> {
	boost::shared_ptr<MeshAnimation> generate(const std::string& id) {
		boost::shared_ptr<OreFileHandle> fh = ResMan::pkg().get_fh(std::string(id));
		
		// Build the parser framework
		xml_schema::string_pimpl string_parser;
		_MeshAnimationParser anim_parser;
		//anim_parser.parsers(0, string_parser);
		
		// Parse the XML data
		xml_schema::document doc_p(anim_parser, "animation");
		anim_parser.pre();
		doc_p.parse(*fh, xsd::cxx::parser::xerces::flags::dont_validate);
		return boost::shared_ptr<MeshAnimation>(anim_parser.post_AnimationType());
	}
};

MeshAnimationCache mesh_animation_cache;

boost::shared_ptr<MeshAnimation> MeshAnimation::create(const std::string& name) {
	return mesh_animation_cache.get(name);
}
