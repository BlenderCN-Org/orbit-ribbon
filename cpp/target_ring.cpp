/*
target_ring.cpp: Implementation of the TargetRing class
TargetRing is a GameObject representing a ring that the player needs to fly through 

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

#include "target_ring.h"

#include <boost/foreach.hpp>
#include <list>

#include "autoxsd/orepkgdesc.h"
#include "constants.h"
#include "debug.h"
#include "globals.h"
#include "mesh.h"

AutoRegistration<GameObjFactorySpec, TargetRingGameObj> target_ring_gameobj_reg("TargetRing");

bool TargetRingGameObj::CheckFaceContactHandler::recent_collision() {
  return (_last_collision != 0) && ((Globals::total_steps - _last_collision)*(float)MAX_FPS < CHECK_FACE_MAX_COLLISION_AGE);
}

bool TargetRingGameObj::CheckFaceContactHandler::handle_collision(
  float t __attribute__ ((unused)),
  dGeomID o __attribute__ ((unused)),
  const dContactGeom* c,
  unsigned int c_len  __attribute__ ((unused))
) {
  Debug::debug_msg("COLLISION!");

  // This geom is never used to create contact joints
  return false;
}

void TargetRingGameObj::step_impl() {
}

void TargetRingGameObj::near_draw_impl() {
  _mesh->draw();
}

TargetRingGameObj::TargetRingGameObj(const ORE1::ObjType& obj) :
  GameObj(obj),
  _mesh(MeshAnimation::load("mesh-LIBTargetRing"))
{
  // Set up geoms for our check faces
  std::list<std::string> face_nums;
  face_nums.push_back("1");
  face_nums.push_back("2");
  BOOST_FOREACH(const std::string& face_num, face_nums) {
    boost::shared_ptr<MeshAnimation> ma = MeshAnimation::load("mesh-" + get_libscene_obj("CheckFace" + face_num).meshName());
    _check_face_meshes.push_back(ma); // Keeps the MeshAnimation from being unloaded until the TargetRing is destroyed
    get_entity().set_geom(
      "check_face_" + face_num,
      dCreateTriMesh(Sim::get_static_space(), ma->get_trimesh_data(0), 0, 0, 0),
      std::auto_ptr<CollisionHandler>(new CheckFaceContactHandler(this))
    );
  }
}
