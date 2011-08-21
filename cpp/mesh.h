/*
mesh.h: Header for the mesh animation classes.
This module handles loading and displaying mesh animations.

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

#ifndef ORBIT_RIBBON_MESH_H
#define ORBIT_RIBBON_MESH_H

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <ode/ode.h>

#include "gameobj.h"

namespace ORE1 { class ObjType; }

class MeshGameObj : public GameObj {
  protected:
    void near_draw_impl();
  
  public:
    MeshGameObj(const ORE1::ObjType& obj);
};

#endif
