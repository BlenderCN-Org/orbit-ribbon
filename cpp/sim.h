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

#include <boost/array.hpp>
#include <boost/utility.hpp>
#include <memory>
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

class OdeEntity;
class Sim {
	public:
		static dWorldID get_ode_world();
		static dSpaceID get_static_space();
		static dSpaceID get_dyn_space();
		
		static std::auto_ptr<OdeEntity> gen_empty_body();
		static std::auto_ptr<OdeEntity> gen_sphere_body(float mass, float rad);
	
	private:
		static void init();
		static void sim_step();
		friend class App;
};

class OdeEntity : boost::noncopyable {
	private:
		friend class Sim;
		
		typedef std::map<std::string, dGeomID> GeomMap;
		
		// These are used when _id isn't set to fill in starting position of newly inserted geoms
		Point _last_pos;
		boost::array<float, 9> _last_rot;
		
		dBodyID _id;
		GeomMap _geoms;
		
		OdeEntity() : _id(0) {}
		OdeEntity(dBodyID id) : _id(id) {}
		
	public:
		bool has_id() const { return _id != 0; }
		dBodyID get_id();
		
		dGeomID get_geom(const std::string& gname);
		void set_geom(const std::string& gname, dGeomID geom);
		
		void set_pos(const Point& pos);
		void set_rot(const boost::array<float, 9>& rot);
		
		~OdeEntity();
};

#endif
