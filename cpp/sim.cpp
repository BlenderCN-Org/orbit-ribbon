#include <GL/gl.h>
#include <ode/ode.h>

#include "app.h"
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

void Sim::_sim_step() {
	dJointGroupEmpty(contact_group);
	
	dWorldQuickStep(ode_world, 1.0f/GLfloat(App::get_max_fps()));
}

