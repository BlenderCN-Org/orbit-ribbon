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
#include "geometry.h"
#include "sim.h"

class MeshAnimation;
namespace ORE1 { class ObjType; }

class AvatarGameObj;
class AvatarGameObj : public GameObj {
	private:
		boost::shared_ptr<MeshAnimation> _anim_fly_to_prerun;
		float _uprightness;
		
		class AvatarContactHandler;
		friend class AvatarContactHandler;
		class AvatarContactHandler : public SimpleContactHandler {
			private:
				AvatarGameObj* _avatar;
				
			public:
				AvatarContactHandler(AvatarGameObj* avatar) : _avatar(avatar) {}
				bool handle_collision(float t, dGeomID o, const dContactGeom* c, unsigned int c_len);
		};
		
		class RunCollisionTracker : public CollisionTracker {
			public:
				bool should_contact(float t, dGeomID o, const dContactGeom* c, unsigned int c_len) const;
		};
		
		bool _run_coll_occurred;
		bool _attached;
		
	protected:
		void step_impl();
		void near_draw_impl();
	
	public:
		AvatarGameObj(const ORE1::ObjType& obj);
};

#endif