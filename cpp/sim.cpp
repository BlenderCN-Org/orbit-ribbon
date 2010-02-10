/*
sim.cpp: Implementation of the Sim class.
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

#include <boost/foreach.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include <ode/ode.h>
#include <vector>

#include "constants.h"
#include "debug.h"
#include "gameobj.h"
#include "globals.h"
#include "sim.h"

dWorldID ode_world;
dSpaceID static_space;
dSpaceID dyn_space;
dJointGroupID contact_group;

const unsigned int MAXIMUM_CONTACT_POINTS = 16;

dWorldID Sim::get_ode_world() {
	return ode_world;
}

dSpaceID Sim::get_static_space() {
	return static_space;
}

dSpaceID Sim::get_dyn_space() {
	return dyn_space;
}

void Sim::init() {
	dInitODE();
	ode_world = dWorldCreate();
	dWorldSetQuickStepNumIterations(ode_world, 10);
	static_space = dHashSpaceCreate(0);
	dyn_space = dHashSpaceCreate(0);
	contact_group = dJointGroupCreate(0);
}

void collision_callback(void* data __attribute__ ((unused)), dGeomID o1, dGeomID o2) {
	static dContactGeom contacts[MAXIMUM_CONTACT_POINTS];
	if (dGeomIsSpace(o1) or dGeomIsSpace(o2)) {
		dSpaceCollide2(o1, o2, NULL, &collision_callback);
	} else {
		Debug::debug_msg("A");
		int len = dCollide(o1, o2, MAXIMUM_CONTACT_POINTS, &(contacts[0]), sizeof(dContactGeom));
		if (len > 0) {
			Debug::debug_msg("B");
			CollisionHandler* o1h = static_cast<CollisionHandler*>(dGeomGetData(o1));
			CollisionHandler* o2h = static_cast<CollisionHandler*>(dGeomGetData(o2));
			o1h->handle_collision(o2, o2h->get_gameobj(), &(contacts[0]), len);
			o2h->handle_collision(o1, o1h->get_gameobj(), &(contacts[0]), len);
			if (o1h->should_contact(o2) && o2h->should_contact(o1)) {
				Debug::debug_msg("C");
				dContact contact;
				contact.surface.mode = dContactApprox1 | dContactBounce;
				contact.surface.bounce = 0.5;
				contact.surface.mu = 5000;
				for (int ci = 0; ci < len; ++ci) {
					contact.geom = contacts[ci];
					dJointID joint = dJointCreateContact(ode_world, contact_group, &contact);
					dJointAttach(joint, dGeomGetBody(o1), dGeomGetBody(o2));
				}
 			}
		}
	}
}

void Sim::sim_step() {
	// Check for collisions
	dJointGroupEmpty(contact_group);
	Debug::debug_msg("DYN -----");
	dSpaceCollide(dyn_space, NULL, &collision_callback); // Collisions among dyn_space objects
	Debug::debug_msg("STATIC -----");
	dSpaceCollide2(dGeomID(dyn_space), dGeomID(static_space), NULL, &collision_callback); // Collisions between dyn_space objects and static_space objects
	Debug::debug_msg("");
	
	// Run the simulation
	dWorldQuickStep(ode_world, 1.0f/MAX_FPS);
	
	// Have each GameObj do whatever it needs to do each step (including damping)
	for (GOMap::iterator i = Globals::gameobjs.begin(); i != Globals::gameobjs.end(); ++i) {
		i->second->step();
	}
}

std::auto_ptr<OdeEntity> Sim::gen_empty_body() {
	return std::auto_ptr<OdeEntity>(new OdeEntity);
}

std::auto_ptr<OdeEntity> Sim::gen_sphere_body(float mass, float rad) {
	dBodyID body = dBodyCreate(ode_world);
	dMass ode_mass;
	dMassSetSphereTotal(&ode_mass, mass, rad);
	dBodySetMass(body, &ode_mass);
	return std::auto_ptr<OdeEntity>(new OdeEntity(body));
}

dBodyID OdeEntity::get_id() {
	if (_id == 0) {
		throw GameException("Attempted to retrieve id from bodyless OdeEntity");
	} else {
		return _id;
	}
}

dGeomID OdeEntity::get_geom(const std::string& gname) {
	std::map<std::string, dGeomID>::const_iterator i = _geoms.find(gname);
	if (i == _geoms.end()) {
		throw GameException("Unable to retrieve geom named " + gname);
	} else {
		return i->second;
	}
}

void gen_ode_rot_matr(const boost::array<float, 9>& rot, dMatrix3 matr) {
	matr[0]  = rot[0]; matr[1]  = rot[3]; matr[2]  = rot[6]; matr[3]  = 0;
	matr[4]  = rot[1]; matr[5]  = rot[4]; matr[6]  = rot[7]; matr[7]  = 0;
	matr[8]  = rot[2]; matr[9]  = rot[5]; matr[10] = rot[8]; matr[11] = 0;
}

void OdeEntity::set_geom(const std::string& gname, dGeomID geom) {
	// TODO possible optimization : to automatically use a space if a body has more than one geom
	// This could be done without changing the external set_geom/get_geom interface
	std::map<std::string, dGeomID>::iterator i = _geoms.find(gname);
	if (i != _geoms.end()) {
		dGeomDestroy(i->second);
		_geoms.erase(i);
	}
	
	if (geom != 0) {
		if (_id != 0) {
			// This automatically sets the geom's position and orientation to the body's
			dGeomSetBody(geom, _id);
		} else {
			dGeomSetPosition(geom, _last_pos.x, _last_pos.y, _last_pos.z);
			dMatrix3 matr;
			gen_ode_rot_matr(_last_rot, matr);
			dGeomSetRotation(geom, matr);
		}
		_geoms[gname] = geom;
	}
}

void OdeEntity::set_pos(const Point& pos) {
	if (_id != 0) {
		dBodySetPosition(_id, pos.x, pos.y, pos.z);
	} else {
		_last_pos = pos;
		BOOST_FOREACH(GeomMap::value_type& p, _geoms) {
			dGeomSetPosition(p.second, pos.x, pos.y, pos.z);
		}
	}
}

void OdeEntity::set_rot(const boost::array<float, 9>& rot) {
	if (_id != 0 || _geoms.size() > 0) {
		// Convert column-major 3x3 to row-major 3x4
		dMatrix3 matr;
		gen_ode_rot_matr(rot, matr);
		if (_id != 0) {
			dBodySetRotation(_id, matr);
		} else {
			_last_rot = rot;
			BOOST_FOREACH(GeomMap::value_type& p, _geoms) {
				dGeomSetRotation(p.second, matr);
			}
		}
	}
}

OdeEntity::~OdeEntity() {
	if (_id != 0) {
		dBodyDestroy(_id);
	}
	
	BOOST_FOREACH(GeomMap::value_type& p, _geoms) {
		dGeomDestroy(p.second);
	}
}