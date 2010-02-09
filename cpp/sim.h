/*
sim.h: Header of the Sim class.
This class is responsible for managing the gameplay simulation and running the ODE stepper.

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

#ifndef ORBIT_RIBBON_SIM_H
#define ORBIT_RIBBON_SIM_H

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <ode/ode.h>

#include "geometry.h"

class App;

class GameObj;
class CollisionHandler {
	public:
		virtual const GameObj* get_gameobj() const { return (GameObj*)0; }
		virtual bool should_contact(dGeomID other) const =0;
		virtual void handle_collision(dGeomID other, const GameObj* other_gameobj, const dContactGeom* contacts, unsigned int contacts_len) =0;
};

class Body;
class Sim {
	public:
		static dWorldID get_ode_world();
		static dSpaceID get_static_space();
		static dSpaceID get_dyn_space();
		
		static boost::shared_ptr<Body> gen_sphere_body(float mass, float rad);
	
	private:
		static void init();
		static void sim_step();
		friend class App;
};

class Body : boost::noncopyable {
	private:
		friend class Sim;
		
		dBodyID _id;
		
		Body(dBodyID id) : _id(id) {}
	
	public:
		dBodyID get_id() { return _id; }
		
		~Body() { dBodyDestroy(_id); }
};

#endif
