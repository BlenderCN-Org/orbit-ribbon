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

#include <boost/lexical_cast.hpp>

#include "autoxsd/orepkgdesc.h"
#include "constants.h"
#include "globals.h"
#include "mesh.h"

AutoRegistration<GameObjFactorySpec, TargetRingGameObj> target_ring_gameobj_reg("TargetRing");

bool TargetRingGameObj::CheckFaceContactHandler::handle_collision(
  float t __attribute__ ((unused)),
  dGeomID o __attribute__ ((unused)),
  const dContactGeom* c __attribute__ ((unused)),
  unsigned int c_len  __attribute__ ((unused))
) {
  // TODO Only care about runners that pass through the ring, not other objects (use ODE bitfield)
  // TODO Use a pair for the map key to track both check face and the object that triggered it
  _target_ring->_check_face_collision_times[this] = Globals::total_steps;

  // This geom is never used to create contact joints
  return false;
}

void TargetRingGameObj::step_impl() { 
  // Destroy any non-recent collisions
  for (std::map<CheckFaceContactHandler*, unsigned int>::iterator i = _check_face_collision_times.begin(); i != _check_face_collision_times.end();) {
    if ((Globals::total_steps - i->second)/(float)MAX_FPS > CHECK_FACE_MAX_COLLISION_AGE) {
      _check_face_collision_times.erase(i++);
    } else {
      ++i;
    }
  }
  
  if (_passed) { return; }

  // If we have a recent collision on every check face, then we can consider that a ring passthru
  if (_check_face_collision_times.size() == CHECK_FACE_COUNT) {
    _passed = true;
  }
}

void TargetRingGameObj::near_draw_impl() {
  _mesh->draw();
}

TargetRingGameObj::TargetRingGameObj(const ORE1::ObjType& obj) :
  GameObj(obj),
  _passed(false),
  _mesh(MeshAnimation::load("mesh-LIBTargetRing"))
{
  // Set up geoms for our check faces
  for (unsigned int i = 1; i <= CHECK_FACE_COUNT; ++i) {
    std::string face_num_str = boost::lexical_cast<std::string>(i);
    const ORE1::ObjType& libscene_obj = get_libscene_obj("CheckFace" + face_num_str);
    boost::shared_ptr<MeshAnimation> ma = MeshAnimation::load("mesh-" + libscene_obj.dataName()); // FIXME Duplicates code in mesh.cpp
    _check_face_meshes.push_back(ma); // Keeps the MeshAnimation from being unloaded until the TargetRing is destroyed
    get_entity().set_geom(
      "check_face_" + face_num_str,
      dCreateTriMesh(Sim::get_static_space(), ma->get_trimesh_data(0), 0, 0, 0),
      std::auto_ptr<CollisionHandler>(new CheckFaceContactHandler(this)),
      &libscene_obj
    );
  }
}
