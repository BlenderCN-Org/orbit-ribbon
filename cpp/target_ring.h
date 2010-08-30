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

class MeshAnimation;
namespace ORE1 { class ObjType; }

class TargetRingGameObj : public GameObj {
  private:
    boost::shared_ptr<MeshAnimation> _mesh;
    boost::shared_ptr<MeshAnimation> _check_mesh_1;
    boost::shared_ptr<MeshAnimation> _check_mesh_2;

  protected:
    void step_impl();
    void near_draw_impl();

  public:
    TargetRingGameObj(const ORE1::ObjType& obj);
};

#endif
