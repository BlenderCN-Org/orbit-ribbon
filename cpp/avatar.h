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

class AvatarGameObj : public GameObj {
	private:
		boost::shared_ptr<MeshAnimation> _anim_fly_to_prerun;
		float _uprightness;
		
		class RunningCollisionHandler : public CollisionHandler {
			private:
				AvatarGameObj* _avatar;
				bool _dirty;
				Vector _normal;
				float _depth;
			
			public:
				RunningCollisionHandler(AvatarGameObj* avatar) : _avatar(avatar), _dirty(false) {}
				
				const GameObj* get_gameobj() const { return _avatar; }
				bool should_contact(dGeomID other __attribute__ ((unused))) const { return false; }
				void handle_collision(dGeomID other, const GameObj* other_gameobj, const dContactGeom* contacts, unsigned int contacts_len);
				
				bool check_dirty();
				Vector get_contact_normal() { return _normal; }
				float get_depth() { return _depth; }
		};
		RunningCollisionHandler _run_coll_handler;
		
		bool _coll_occurred;
		
	protected:
		void step_impl();
		void near_draw_impl();
	
	public:
		AvatarGameObj(const ORE1::ObjType& obj);
};

#endif