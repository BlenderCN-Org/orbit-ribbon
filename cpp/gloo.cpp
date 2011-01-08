/*
gloo.cpp: Implementation of GL Object-Oriented utility classes.

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

#include <memory>
#include <list>
#include <string>
#include <sstream>
#include <GL/glew.h>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <ode/ode.h>

#include "cache.h"
#include "constants.h"
#include "except.h"
#include "geometry.h"
#include "globals.h"
#include "gloo.h"
#include "ore.h"
#include "sim.h"

#include "debug.h"

// How many kilobytes should be allocated for the vertices buffer and faces buffer respectively
const int VERTICES_BUFFER_ALLOCATED_SIZE = 4096;
const int FACES_BUFFER_ALLOCATED_SIZE = 2048;

GLOOPushedMatrix::GLOOPushedMatrix() {
  glPushMatrix();
}

GLOOPushedMatrix::~GLOOPushedMatrix() {
  glPopMatrix();
}

struct SDLSurf {
  SDL_Surface* s;  
  SDLSurf(SDL_Surface* i) : s(i) {
    if (!i) throw GameException(std::string("Error while loading texture surface: ") + SDL_GetError());
    SDL_LockSurface(s);
  }
  ~SDLSurf() { SDL_UnlockSurface(s); SDL_FreeSurface(s); }
  SDL_Surface* operator->() { return s; }
  SDL_Surface& operator*() { return *s; }
};

boost::shared_ptr<GLOOTexture> GLOOTexture::TextureCache::generate(const std::string& id) {
  try {
    // Create an SDL surface from the requested ORE image file
    boost::shared_ptr<OreFileHandle> fh = Globals::ore->get_fh(std::string("image-") + id + ".tga");
    SDL_RWops rwops = fh->get_sdl_rwops();
    return boost::shared_ptr<GLOOTexture>(new GLOOTexture(rwops));
  } catch (const std::exception& e) {
    throw GameException("Unable to load texture " + id + " : " + e.what());
  }
}

GLOOTexture::TextureCache GLOOTexture::_cache;

GLOOTexture::GLOOTexture(SDL_RWops& rwops, bool alpha_tex) {
  SDLSurf surf(IMG_LoadTGA_RW(&rwops)); //SDLSurf takes care of locking surface now and later unlocking/freeing it
  if (!(
    surf->format->BitsPerPixel == 8 || (
      surf->format->Bshift == 0 &&
      surf->format->Gshift == 8 &&
      surf->format->Rshift == 16 &&
      (surf->format->BitsPerPixel == 24 || surf->format->Ashift == 24)
    )
    )) {
    throw GameException(
      std::string("Unknown pixel format :") +
      " BPP " + boost::lexical_cast<std::string>((unsigned int)surf->format->BitsPerPixel) +
      " Rs " + boost::lexical_cast<std::string>((unsigned int)surf->format->Rshift) + 
      " Gs " + boost::lexical_cast<std::string>((unsigned int)surf->format->Gshift) +
      " Bs " + boost::lexical_cast<std::string>((unsigned int)surf->format->Bshift) +
      " As " + boost::lexical_cast<std::string>((unsigned int)surf->format->Ashift)
    );
  }

  _width = surf->w;
  _height = surf->h;
  glGenTextures(1, &(_tex_name));
  glBindTexture(GL_TEXTURE_2D, _tex_name);
  if (surf->format->BitsPerPixel == 8) {
    if (alpha_tex) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_ALPHA, surf->w, surf->h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, surf->pixels);
    } else {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_LUMINANCE, surf->w, surf->h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, surf->pixels);
    }
    _mipmapped = false;
  } else {
    if (alpha_tex) {
      throw GameException("Cannot use multi-channel texture as alpha texture");
    }
    GLenum img_format = (surf->format->BitsPerPixel == 24 ? GL_BGR : GL_BGRA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA, surf->w, surf->h, 0, img_format, GL_UNSIGNED_BYTE, surf->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    _mipmapped = true;
  }

  if (glGetError()) {
    throw GameException("GL error while loading texture");
  }
}

boost::shared_ptr<GLOOTexture> GLOOTexture::load(const std::string& name) {
  return _cache.get(name);
}

void GLOOTexture::bind() {
  glBindTexture(GL_TEXTURE_2D, _tex_name);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _mipmapped ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR);
}

void GLOOTexture::draw_2d(const Point& pos) {
  const static GLfloat uv_points[8] = {
    0.0, 1.0,
    1.0, 1.0,
    1.0, 0.0,
    0.0, 0.0
  };
  
  GLfloat points[8] = {
    pos.x, pos.y + _height,
    pos.x + _width, pos.y + _height,
    pos.x + _width, pos.y,
    pos.x, pos.y
  };
  
  bind();
  
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDisableClientState(GL_NORMAL_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, points);
  glTexCoordPointer(2, GL_FLOAT, 0, uv_points);
  glDrawArrays(GL_QUADS, 0, 8);
  glPopClientAttrib();
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

GLOOTexture::~GLOOTexture() {
  glDeleteTextures(1, &_tex_name);
}

void GLOOVertex::set_gl_vbo_pointers() {
  const char* offset = 0;
  glVertexPointer(3, GL_FLOAT, sizeof(GLOOVertex), offset);
  glNormalPointer(GL_FLOAT, sizeof(GLOOVertex), offset + 3*sizeof(float));
  glClientActiveTexture(GL_TEXTURE0);
  glTexCoordPointer(2, GL_FLOAT, sizeof(GLOOVertex), offset + 6*sizeof(float));
}

_VBOManager::Allocation::Allocation(unsigned int bytes, _VBOManager* man) :
  _man(man)
{
  AllocRange rng;
  rng.bytes = bytes;
  rng.offset = 0;
  if (!_man->_ranges.empty()) {
    rng.offset = _man->_ranges.back().offset + _man->_ranges.back().bytes;
  }
  if (rng.offset + rng.bytes > _man->_max_bytes) {
    throw OreException("Ran out of space while attempting allocation within VBO!");
  }
  _man->_ranges.push_back(rng);
  _iter = _man->_ranges.end();
  --_iter;
}

void* _VBOManager::Allocation::map() {
  if (_man->_mapped) {
    throw OreException("Attempted to map memory within already-mapped VBO");
  } else {
    _man->_mapped = true;
    return glMapBufferRange(_man->_tgt, offset(), bytes(), GL_MAP_WRITE_BIT);
  }
}

void _VBOManager::Allocation::unmap() {
  if (_man->_mapped) {
    GLenum result = glUnmapBuffer(_man->_tgt);
    if (result == GL_FALSE) {
      throw OreException("Memory corruption during VBO unmap");
    }
    _man->_mapped = false;
  }
}

_VBOManager::Allocation::~Allocation() {
  unmap();
  _man->_ranges.erase(_iter);
}

_VBOManager::_VBOManager(GLenum tgt, unsigned int max_bytes) :
  _tgt(tgt),
  _max_bytes(max_bytes),
  _mapped(false)
{
  glGenBuffers(1, &_buf_id);
  glBindBuffer(tgt, _buf_id);
  glBufferData(tgt, max_bytes, NULL, GL_STATIC_DRAW);
}

_VBOManager::~_VBOManager() {
  glDeleteBuffers(1, &_buf_id);
}

float _VBOManager::get_usage() const {
  float used = 0.0;
  if (!_ranges.empty()) {
    used = float(_ranges.back().offset + _ranges.back().bytes);
  }
  return used/float(_max_bytes);
}

bool GLOOBufferedMesh::_initialized = false;
boost::scoped_ptr<_VBOManager> GLOOBufferedMesh::_vertices_vboman, GLOOBufferedMesh::_faces_vboman;
std::map<dTriMeshDataID, const GLOOBufferedMesh*> GLOOBufferedMesh::_trimesh_id_map;

const GLOOBufferedMesh* GLOOBufferedMesh::get_mesh_from_geom(dGeomID g) {
  if (dGeomGetClass(g) != dTriMeshClass) {
    return 0;
  }
  
  dTriMeshDataID d = dGeomTriMeshGetData(g);
  std::map<dTriMeshDataID, const GLOOBufferedMesh*>::iterator i = _trimesh_id_map.find(d);
  if (i == _trimesh_id_map.end()) {
    return 0;
  } else {
    return i->second;
  }
}

GLOOBufferedMesh::GLOOBufferedMesh(unsigned int vertex_count, unsigned int face_count, boost::shared_ptr<GLOOTexture> tex, bool gen_trimesh_data) :
  _trimesh_id(0),
  _tex(tex),
  _vertices_added(0),
  _total_vertices(vertex_count),
  _faces_added(0),
  _total_faces(face_count)
{
  if (!_initialized) {
    _vertices_vboman.reset(new _VBOManager(GL_ARRAY_BUFFER, VERTICES_BUFFER_ALLOCATED_SIZE*1024));
    _faces_vboman.reset(new _VBOManager(GL_ELEMENT_ARRAY_BUFFER, FACES_BUFFER_ALLOCATED_SIZE*1024));
    GLOOVertex::set_gl_vbo_pointers();
    _initialized = true;
  }
  
  if (gen_trimesh_data) {
    _trimesh_vertices.reserve(3*vertex_count);
    _trimesh_normals.reserve(3*vertex_count);
    _trimesh_indices.reserve(3*face_count);
  }
  
  _vertices_alloc = _vertices_vboman->allocate(_total_vertices*sizeof(GLOOVertex));
  _next_vertex = (GLOOVertex*)_vertices_alloc->map();
  
  _faces_alloc = _faces_vboman->allocate(_total_faces*sizeof(GLOOFace));
  _next_face = (GLOOFace*)_faces_alloc->map();
}

void GLOOBufferedMesh::load_vertex(const GLOOVertex& v) {
  if (_vertices_added >= _total_vertices) {
    throw OreException("Attempted to load more vertices than space was allocated for");
  }
  if (!_next_vertex) {
    throw OreException("Attempted to load a vertex while unmapped");
  }
  if (_trimesh_vertices.capacity() > 0) {
    _trimesh_vertices.push_back(v.x);
    _trimesh_vertices.push_back(v.y);
    _trimesh_vertices.push_back(v.z);
    _trimesh_normals.push_back(v.nx);
    _trimesh_normals.push_back(v.ny);
    _trimesh_normals.push_back(v.nz);
  }
  *_next_vertex = v;
  ++_next_vertex;
  ++_vertices_added;
}

void GLOOBufferedMesh::load_face(const GLOOFace& f) {
  if (_faces_added >= _total_faces) {
    throw OreException("Attempted to load more faces than space was allocated for");
  }
  if (!_next_face) {
    throw OreException("Attempted to load a face while unmapped");
  }
  if (_trimesh_indices.capacity() > 0) {
    _trimesh_indices.push_back(f.a);
    _trimesh_indices.push_back(f.b);
    _trimesh_indices.push_back(f.c);
  }
  unsigned int vtx_offset = (_vertices_alloc->offset()/sizeof(GLOOVertex));
  _next_face->a = f.a + vtx_offset;
  _next_face->b = f.b + vtx_offset;
  _next_face->c = f.c + vtx_offset;
  ++_next_face;
  ++_faces_added;
}

void GLOOBufferedMesh::finish_loading() {
  if (_next_vertex == 0 && _next_face == 0) {
    return;
  }
  
  if (_vertices_added != _total_vertices) {
    throw OreException((boost::format("Allocated space for %1% vertices, but only loaded %2%") % _total_vertices % _vertices_added).str());
  }
  
  if (_faces_added != _total_faces) {
    throw OreException((boost::format("Allocated space for %1% faces, but only loaded %2%") % _total_faces % _faces_added).str());
  }
  
  if (_trimesh_vertices.capacity() > 0) {
    _trimesh_id = dGeomTriMeshDataCreate();
    dGeomTriMeshDataBuildDouble1(_trimesh_id,
      &*(_trimesh_vertices.begin()), 3*sizeof(dReal), _total_vertices,
      &*(_trimesh_indices.begin()), _total_faces*3, 3*sizeof(unsigned int),
      &*(_trimesh_normals.begin()));
    _trimesh_id_map[_trimesh_id] = this;
  }
  
  _vertices_alloc->unmap();
  _next_vertex = 0;
  _faces_alloc->unmap();
  _next_face = 0;
}

void GLOOBufferedMesh::draw() {
  if (_vertices_vboman->mapped() or _faces_vboman->mapped()) {
    throw OreException("Attempted to draw GLOOBufferedMesh while still mapped");
  }
  
  if (_tex) {
    _tex->bind();
  } else {
    glDisable(GL_TEXTURE_2D);
  }
  
  glDrawRangeElements(
    GL_TRIANGLES,
    (_vertices_alloc->offset())/sizeof(GLOOVertex),
    (_vertices_alloc->offset() + _vertices_alloc->bytes())/sizeof(GLOOVertex),
    _faces_alloc->bytes()/GLOOFace::elem_bytes(),
    GLOOFace::gl_type(),
    (const char*)(0) + _faces_alloc->offset()
  );
  
  if (!_tex) {
    glEnable(GL_TEXTURE_2D);
  }
}

dTriMeshDataID GLOOBufferedMesh::get_trimesh_data() const {
  if (_vertices_vboman->mapped() or _faces_vboman->mapped()) {
    throw OreException("Attempted to get trimesh data from GLOOBufferedMesh while still mapped");
  }
  return _trimesh_id;
}

Point GLOOBufferedMesh::get_vertex_pos(unsigned int v_idx) const {
  return Point(
    _trimesh_vertices[v_idx*3],
    _trimesh_vertices[v_idx*3 + 1],
    _trimesh_vertices[v_idx*3 + 2]
  );
}

Vector GLOOBufferedMesh::get_vertex_norm(unsigned int v_idx) const {
  return Vector(
    _trimesh_normals[v_idx*3],
    _trimesh_normals[v_idx*3 + 1],
    _trimesh_normals[v_idx*3 + 2]
  );
}

GLOOFace GLOOBufferedMesh::get_face(unsigned int f_idx) const {
  return GLOOFace(
    _trimesh_indices[f_idx*3],
    _trimesh_indices[f_idx*3 + 1],
    _trimesh_indices[f_idx*3 + 2]
  );
}

Vector GLOOBufferedMesh::get_interpolated_normal(dGeomID g, const Point& p, unsigned int f_idx) const {
  GLOOFace face = get_face(f_idx);
  Vector normals[3] = { get_vertex_norm(face.a), get_vertex_norm(face.b), get_vertex_norm(face.c) };
  Vector b = get_barycentric(OdeGeomUtil::get_pos_rel_point(g, p), get_vertex_pos(face.a), get_vertex_pos(face.b), get_vertex_pos(face.c));
  return OdeGeomUtil::vector_to_world(g, normals[0]*b[0] + normals[1]*b[1] + normals[2]*b[2]);
}

GLOOBufferedMesh::~GLOOBufferedMesh() {
  if (_trimesh_id != 0) {
    _trimesh_id_map.erase(_trimesh_id);
    dGeomTriMeshDataDestroy(_trimesh_id);
  }
  finish_loading();
}

void GLOOCamera::setup() const {
  gluLookAt(
    pos.x, pos.y, pos.z,
    tgt.x, tgt.y, tgt.z,
     up.x,  up.y,  up.z
  );
}

void GLOOCamera::operator*=(float n) {
  pos *= n;
  tgt *= n;
  up *= n;
}

void GLOOCamera::operator+=(const GLOOCamera& other) {
  pos += other.pos;
  tgt += other.tgt;
  up += other.up;
}
