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

#include <GL/glew.h>
#include <GL/gl.h>
#include <ode/ode.h>

#include "constants.h"
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
		int len = dCollide(o1, o2, MAXIMUM_CONTACT_POINTS, &(contacts[0]), sizeof(dContactGeom));
		if (len > 0) {
			CollisionHandler* o1h = static_cast<CollisionHandler*>(dGeomGetData(o1));
			CollisionHandler* o2h = static_cast<CollisionHandler*>(dGeomGetData(o2));
			o1h->handle_collision(o2, o2h->get_gameobj(), &(contacts[0]), len);
			o2h->handle_collision(o1, o1h->get_gameobj(), &(contacts[0]), len);
			if (o1h->should_contact(o2) && o2h->should_contact(o1)) {
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
	dSpaceCollide(dyn_space, NULL, &collision_callback); // Collisions among dyn_space objects
	dSpaceCollide2(dGeomID(dyn_space), dGeomID(static_space), NULL, &collision_callback); // Collisions between dyn_space objects and static_space objects
	
	// Run the simulation
	dWorldQuickStep(ode_world, 1.0f/MAX_FPS);
	
	// Have each GameObj do whatever it needs to do each step (including damping)
	for (GOMap::iterator i = Globals::gameobjs.begin(); i != Globals::gameobjs.end(); ++i) {
		i->second->step();
	}
}

dBodyID Sim::gen_sphere_body(float mass, float rad) {
	dBodyID body = dBodyCreate(ode_world);
	dMass ode_mass;
	dMassSetSphereTotal(&ode_mass, mass, rad);
	dBodySetMass(body, &ode_mass);
	return body;
}
