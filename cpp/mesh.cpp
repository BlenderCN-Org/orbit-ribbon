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
#include "gloo.h"
#include "mesh.h"
#include "resman.h"

class _MeshAnimationParser : public ORE1::AnimationType_pskel {
	private:
		boost::shared_ptr<MeshAnimation> _anim_p;
	
	public:
		void pre() {
			_anim_p = boost::shared_ptr<MeshAnimation>(new MeshAnimation());
		}
		
		void frame() {
		}
		
		void name(const std::string& n) {
			_anim_p->name = n;
		}
		
		boost::shared_ptr<MeshAnimation> post_AnimationType() {
			return _anim_p;
		}
};

class _MeshParser : public ORE1::MeshType_pskel {
	private:
	
	public:
		void pre() {
		}
		
		void f(GLOOFace* face) {
		}
		
		void v(GLOOVertex* v) {
		}
		
		void texture(const std::string& tex_name) {
		}
		
		void vertcount(unsigned int c) {
		}
		
		void facecount(unsigned int c) {
		}
		
		boost::shared_ptr<_Mesh> post_MeshType() {
		}
};

class _FaceParser : public ORE1::FaceType_pskel {
	private:
	
	public:
		void item(unsigned int i) {
		}
		
		GLOOFace* post_FaceType() {
		}
};

class _VertexParser : public ORE1::VertexType_pskel {
	private:
		
	public:
		void p(boost::array<GLfloat,3>* pos) {
		}
		
		void n(boost::array<GLfloat,3>* normal) {
		}
		
		void t(boost::array<GLfloat,2>* uv) {
		}
		
		GLOOVertex* post_VertexType() {
		}
};

class _PosParser : public ORE1::Coord3DType_pskel {
	private:
	
	public:
		void item(float i) {
		}
		
		boost::array<GLfloat,3>* post_Coord3DType() {
		}
};

class _NormalParser : public ORE1::Coord3DType_pskel {
	private:
	
	public:
		void item(float i) {
		}
		
		boost::array<GLfloat,3>* post_Coord3DType() {
		}
};

class _UvParser : public ORE1::Coord2DType_pskel {
	private:
	
	public:
		void item(float i) {
		}
		
		boost::array<GLfloat,2>* post_Coord2DType() {
		}
};

class MeshAnimationCache : public CacheBase<MeshAnimation> {
	boost::shared_ptr<MeshAnimation> generate(const std::string& id) {
		boost::shared_ptr<OreFileHandle> fh = ResMan::pkg().get_fh(std::string(id));
		
		// Build the parser framework
		xml_schema::string_pimpl string_parser;
		xml_schema::unsigned_int_pimpl uint_parser;
		xml_schema::float_pimpl float_parser;
		_MeshAnimationParser anim_parser;
		_MeshParser mesh_parser;
		_FaceParser face_parser;
		_VertexParser vertex_parser;
		_PosParser pos_parser;
		_NormalParser normal_parser;
		_UvParser uv_parser;
		
		anim_parser.parsers(mesh_parser, string_parser);
		mesh_parser.parsers(face_parser, vertex_parser, string_parser, uint_parser, uint_parser);
		face_parser.parsers(uint_parser);
		vertex_parser.parsers(pos_parser, normal_parser, uv_parser);
		pos_parser.parsers(float_parser);
		normal_parser.parsers(float_parser);
		uv_parser.parsers(float_parser);
		
		// Parse the XML data
		xml_schema::document doc_p(anim_parser, "animation");
		anim_parser.pre();
		doc_p.parse(*fh, xsd::cxx::parser::xerces::flags::dont_validate);
		return anim_parser.post_AnimationType();
	}
};

MeshAnimationCache mesh_animation_cache;

boost::shared_ptr<MeshAnimation> MeshAnimation::create(const std::string& name) {
	return mesh_animation_cache.get(name);
}
