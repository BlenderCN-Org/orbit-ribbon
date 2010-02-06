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

#include <ode/ode.h>
#include <boost/shared_ptr.hpp>

#include "gameobj.h"
#include "sim.h"

class MeshAnimation;
namespace ORE1 { class ObjType; }

class AvatarGameObj : public GameObj {
	public:
		enum Mode {Superman, SupermanToUpright, Upright, UprightToSuperman, Attached};
		
	private:
		boost::shared_ptr<MeshAnimation> _anim_fly_to_prerun;
		Mode _mode;
		unsigned int _mode_entry_time;
		
		float get_stance(); // If 0.0, body is oriented along z axis. If 1.0, body is oriented along y axis.
		void set_mode(Mode mode);
		
	protected:
		void step_impl();
		void near_draw_impl();
	
	public:
		AvatarGameObj(const ORE1::ObjType& obj);
};

#endif