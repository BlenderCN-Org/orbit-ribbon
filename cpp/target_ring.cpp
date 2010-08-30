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

#include "autoxsd/orepkgdesc.h"
#include "debug.h"
#include "globals.h"
#include "mesh.h"

AutoRegistration<GameObjFactorySpec, TargetRingGameObj> target_ring_gameobj_reg("TargetRing");

void TargetRingGameObj::step_impl() {
}

void TargetRingGameObj::near_draw_impl() {
  _mesh->draw();
}

TargetRingGameObj::TargetRingGameObj(const ORE1::ObjType& obj) :
  GameObj(obj),
  _mesh(MeshAnimation::load("mesh-LIBTargetRing"))
{
  /*
  Debug::debug_msg("LOADING...");
  const ORE1::SubsceneType& libscene = Globals::libscenes.find("TargetRing")->second;
  for (ORE1::SubsceneType::ObjConstIterator i = libscene.obj().begin(); i != libscene.obj().end(); ++i) {
    Debug::debug_msg("SUBSCENE OBJ " + i->objName());
  }
  */
}
