/*
target_ring.h: Header of the TargetRing class
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

#ifndef ORBIT_RIBBON_TARGET_RING_H
#define ORBIT_RIBBON_TARGET_RING_H

#include <boost/shared_ptr.hpp>

#include "gameobj.h"
#include "sim.h"

// Number of seconds before a collision with a check face is no longer considered recent enough to count
// This way we can reduce the chance that the player can just whiff past both checkfaces from the outside
const float CHECK_FACE_MAX_COLLISION_AGE = 1.0; 

class MeshAnimation;
namespace ORE1 { class ObjType; }

class TargetRingGameObj;
class TargetRingGameObj : public GameObj {
  private:
    boost::shared_ptr<MeshAnimation> _mesh;
    boost::shared_ptr<GameObj> _check_face_1;
    boost::shared_ptr<GameObj> _check_face_2;

    class CheckFaceContactHandler : public SimpleContactHandler {
      private:
        TargetRingGameObj* _target_ring;
        unsigned int _last_collision;

      public:
        CheckFaceContactHandler(TargetRingGameObj* target_ring) : _target_ring(target_ring), _last_collision(0) {}
        bool recent_collision();
        bool handle_collision(float t, dGeomID o, const dContactGeom* c, unsigned int c_len);
    };

  protected:
    void step_impl();
    void near_draw_impl();

  public:
    TargetRingGameObj(const ORE1::ObjType& obj);
};

#endif
