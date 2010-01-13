/*
avatar.h: Header of the Avatar class
Avatar is a GameObject representing the player character

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

#ifndef ORBIT_RIBBON_AVATAR_H
#define ORBIT_RIBBON_AVATAR_H

#include "gameobj.h"

namespace ORE1 { class ObjType; }

class AvatarGameObj : public GameObj {
	protected:
		void step_impl();
		void near_draw_impl();
	
	public:
		AvatarGameObj(const ORE1::ObjType& obj);
};

#endif