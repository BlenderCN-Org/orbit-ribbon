#include <GL/gl.h>
#include <ode/ode.h>

#include "constants.h"
#include "sim.h"

dWorldID ode_world;
dSpaceID static_space;
dSpaceID dyn_space;
dJointGroupID contact_group;

dWorldID Sim::get_ode_world() {
	return ode_world;
}

dSpaceID Sim::get_static_space() {
	return static_space;
}

dSpaceID Sim::get_dyn_space() {
	return dyn_space;
}

void Sim::_init() {	
	ode_world = dWorldCreate();
	dWorldSetQuickStepNumIterations(ode_world, 10);
	static_space = dHashSpaceCreate(0);
	dyn_space = dHashSpaceCreate(0);
	contact_group = dJointGroupCreate(0);
}

void collision_callback(void* data, dGeomID o1, dGeomID o2) {
}

void Sim::_sim_step() {
	// Check for collisions
	dJointGroupEmpty(contact_group);
	dSpaceCollide(dyn_space, NULL, &collision_callback); // Collisions among dyn_space objects
	dSpaceCollide2(dGeomID(dyn_space), dGeomID(static_space), NULL, &collision_callback); // Collisions between dyn_space objects and static_space objects
	
	// Run the simulation
	dWorldQuickStep(ode_world, 1.0f/MAX_FPS);
}

