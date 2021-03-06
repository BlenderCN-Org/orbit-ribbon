/*
mesh.cpp: Implementation for mesh and animation classes.
This module handles loading and displaying meshes, animations, and textures

Copyright 2011 David Simon <david.mike.simon@gmail.com>

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
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>

#include "autoxsd/orepkgdesc.h"
#include "autoxsd/oreanim-pskel.h"
#include "cache.h"
#include "debug.h"
#include "globals.h"

#include "mesh.h"
#include "ore.h"
#include "sim.h"

class _MeshAnimationParser : public OREAnim1::AnimationType_pskel {
  private:
    boost::shared_ptr<MeshAnimation> _anim_p;
  
  public:
    void pre() {
      _anim_p = boost::shared_ptr<MeshAnimation>(new MeshAnimation());
    }
    
    void frame(boost::shared_ptr<GLOOBufferedMesh> mesh) {
      _anim_p->_frames.push_back(mesh);
    }
    
    void name(const std::string& n) {
      _anim_p->_name = n;
    }
    
    boost::shared_ptr<MeshAnimation> post_AnimationType() {
      boost::shared_ptr<MeshAnimation> ret = _anim_p;
      _anim_p.reset();
      return ret;
    }
};

class _MeshParser : public OREAnim1::MeshType_pskel {
  private:
    boost::shared_ptr<GLOOBufferedMesh> _mesh;
    unsigned int _verts, _faces;
    std::string _tex_name;
    
    void init_mesh() {
      if (!_mesh) {
        if (_verts == 0 || _faces == 0) {
          throw OreException("Unable to create GLOOBufferedMesh without allocation attributes");
        }
        boost::shared_ptr<GLOOTexture> _tex;
        if (_tex_name.size() > 0) {
          _tex = GLOOTexture::load(_tex_name);
        }
        // FIXME Don't always have trimesh data creaed; it's not necessary for all models
        _mesh = GLOOBufferedMesh::create(_verts, _faces, _tex, true);
      }
    }
  
  public:
    void pre() {
      _mesh.reset();
      _verts = _faces = 0;
      _tex_name = "";
    }
    
    void f(GLOOFace* face) {
      init_mesh();
      _mesh->load_face(*face);
    }
    
    void v(GLOOVertex* v) {
      init_mesh();
      _mesh->load_vertex(*v);
    }
    
    void texture(const std::string& tex_name) {
      _tex_name = tex_name;
    }
    
    void vertcount(unsigned int c) {
      _verts = c;
    }
    
    void facecount(unsigned int c) {
      _faces = c;
    }
    
    boost::shared_ptr<GLOOBufferedMesh> post_MeshType() {
      boost::shared_ptr<GLOOBufferedMesh> ret = _mesh;
      _mesh.reset();
      ret->finish_loading();
      return ret;
    }
};

class _FaceParser : public OREAnim1::FaceType_pskel {
  private:
    GLOOFace _face;
    unsigned int _idx;
  
  public:
    _FaceParser() : OREAnim1::FaceType_pskel(0) { }

    void pre() {
      _idx = 0;
    }
    
    void item(unsigned int i) {
      switch (_idx) {
        case 0:
          _face.a = i;
          break;
        case 1:
          _face.b = i;
          break;
        case 2:
          _face.c = i;
          break;
        default:
          throw OreException("Too many indices supplied for a face");
      }
      ++_idx;
    }
    
    GLOOFace* post_FaceType() {
      return &_face;
    }
};

class _VertexParser : public OREAnim1::VertexType_pskel {
  private:
    GLOOVertex _vert;
    
  public:
    void p(boost::array<float,3>* pos) {
      _vert.x = pos->at(0);
      _vert.y = pos->at(1);
      _vert.z = pos->at(2);
    }
    
    void n(boost::array<float,3>* normal) {
      _vert.nx = normal->at(0);
      _vert.ny = normal->at(1);
      _vert.nz = normal->at(2);
    }
    
    void t(boost::array<float,2>* uv) {
      _vert.u = uv->at(0);
      _vert.v = uv->at(1);
    }
    
    GLOOVertex* post_VertexType() {
      return &_vert;
    }
};

class _Coord3DParser : public OREAnim1::Coord3DType_pskel {
  private:
    boost::array<float,3> _data;
    unsigned int _idx;
  
  public:
    _Coord3DParser() : OREAnim1::Coord3DType_pskel(0) { }
    
    void pre() {
      _idx = 0;
    }
    
    void item(float i) {
      _data[_idx] = i;
      ++_idx;
    }
    
    boost::array<float,3>* post_Coord3DType() {
      return &_data;
    }
};

class _UvParser : public OREAnim1::Coord2DType_pskel {
  private:
    boost::array<float,2> _data;
    unsigned int _idx;
  
  public:
    _UvParser() : OREAnim1::Coord2DType_pskel(0) { }
    
    void pre() {
      _idx = 0;
    }
    
    void item(float i) {
      _data[_idx] = i;
      ++_idx;
    }
    
    boost::array<float,2>* post_Coord2DType() {
      return &_data;
    }
};

class _MeshParsingRig {
  private:
    xml_schema::string_pimpl string_parser;
    xml_schema::unsigned_int_pimpl uint_parser;
    xml_schema::float_pimpl float_parser;
    _MeshAnimationParser anim_parser;
    _MeshParser mesh_parser;
    _FaceParser face_parser;
    _VertexParser vertex_parser;
    _Coord3DParser coord3d_parser;
    _UvParser uv_parser;
  
  public:
    _MeshParsingRig() {
      anim_parser.parsers(string_parser, mesh_parser);
      mesh_parser.parsers(string_parser, uint_parser, uint_parser, face_parser, vertex_parser);
      face_parser.parsers(uint_parser);
      vertex_parser.parsers(coord3d_parser, coord3d_parser, uv_parser);
      coord3d_parser.parsers(float_parser);
      uv_parser.parsers(float_parser);
    }
    
    boost::shared_ptr<MeshAnimation> parse(OreFileHandle& fh) {
      xml_schema::document_pimpl doc_p(anim_parser, "http://www.orbit-ribbon.org/OREAnim1", "animation");
      anim_parser.pre();
      doc_p.parse(fh);
      return anim_parser.post_AnimationType();
    }
};

_MeshParsingRig parsing_rig;

class MeshAnimationCache : public CacheBase<MeshAnimation> {
  boost::shared_ptr<MeshAnimation> generate(const std::string& id) {
    boost::shared_ptr<OreFileHandle> fh = Globals::ore->get_fh(id);
    try {
      return parsing_rig.parse(*fh);
    } catch (const std::exception& e) {
      throw GameException("Unable to parse MeshAnimation " + id + " : " + e.what());
    }
  }
};

MeshAnimationCache mesh_animation_cache;

boost::shared_ptr<MeshAnimation> MeshAnimation::load(const std::string& name) {
  return mesh_animation_cache.get(name);
}

void MeshAnimation::draw() {
  //FIXME Advance through the frames
  if (_frames.size() == 0) {
    throw GameException("No frames loaded in animation '" + _name + "'!");
  }
  _frames[0]->draw();
}

void MeshGameObj::near_draw_impl() {
  _mesh_anim->draw();
}

// Set MeshGameObj as the default type for unknown GameObjs
AutoDefaultRegistration<GameObjFactorySpec, MeshGameObj> mesh_gameobj_reg;

MeshGameObj::MeshGameObj(const ORE1::ObjType& obj) :
  GameObj(obj),
  _mesh_anim(MeshAnimation::load(std::string("mesh-") + obj.dataName()))
{
  get_entity().set_geom(
    "physical",
    dCreateTriMesh(Sim::get_static_space(), _mesh_anim->get_trimesh_data(0), 0, 0, 0),
    std::auto_ptr<CollisionHandler>(new SimpleContactHandler)
  );
}
