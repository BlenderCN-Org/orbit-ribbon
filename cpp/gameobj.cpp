#include <GL/gl.h>
#include <ode/ode.h>
#include <boost/array.hpp>

#include "constants.h"
#include "gameobj.h"
#include "geometry.h"
#include "gloo.h"

GameObj::GameObj(const Point& npos, const boost::array<GLfloat, 9>& nrot) :
	pos(npos),
	rot(nrot),
	vel(Vector()),
	body(0),
	geom(0)
{
	vel_damp_coef[0] = DEFAULT_VEL_DAMP_COEF;
	vel_damp_coef[1] = DEFAULT_VEL_DAMP_COEF;
	vel_damp_coef[2] = DEFAULT_VEL_DAMP_COEF;
	
	ang_damp_coef[0] = DEFAULT_ANG_DAMP_COEF;
	ang_damp_coef[1] = DEFAULT_ANG_DAMP_COEF;
	ang_damp_coef[2] = DEFAULT_ANG_DAMP_COEF;
}

GameObj::~GameObj() {
	set_body(0);
	set_geom(0);
}

void recursive_geom_set_pos(dGeomID g, const Point& npos) {
	if (dGeomIsSpace(g)) {
		dSpaceID s = dSpaceID(g);
		GLint end = dSpaceGetNumGeoms(s);
		for (int i = 0; i < end; ++i) {
			recursive_geom_set_pos(dSpaceGetGeom(s, i), npos);
		}
	} else {
		dGeomSetPosition(g, npos.x, npos.y, npos.z);
	}
}

void GameObj::set_pos(const Point& npos) {
	pos = npos;
	
	if (body != 0) {
		dBodySetPosition(body, pos.x, pos.y, pos.z);
	} else if (geom != 0) {
		recursive_geom_set_pos(geom, pos);
	}
}

void recursive_geom_set_rot(dGeomID g, const dMatrix3& matr) {
	if (dGeomIsSpace(g)) {
		dSpaceID s = dSpaceID(g);
		GLint end = dSpaceGetNumGeoms(s);
		for (int i = 0; i < end; ++i) {
			recursive_geom_set_rot(dSpaceGetGeom(s, i), matr);
		}
	} else {
		dGeomSetRotation(g, matr);
	}
}

void GameObj::set_rot(const boost::array<GLfloat, 9>& nrot) {
	rot = nrot;
	
	// Convert column-major to row-major
	dMatrix3 matr;
	matr[0] = nrot[0]; matr[1] = nrot[3]; matr[2] = nrot[6];
	matr[3] = nrot[1]; matr[4] = nrot[4]; matr[5] = nrot[7];
	matr[6] = nrot[2]; matr[7] = nrot[5]; matr[8] = nrot[8];
	if (body != 0) {
		dBodySetRotation(body, matr);
	} else if (geom != 0) {
		recursive_geom_set_rot(geom, matr);
	}
}

std::string GameObj::to_str() const {
	// FIXME : Implement
	return std::string("");
}

void GameObj::draw(bool near) {
	GLOOPushedMatrix pm;
	glTranslatef(pos.x, pos.y, pos.z);
	// FIXME : Apply rotation matrix here
	
	if (near) {
		near_draw_impl();
	} else {
		far_draw_impl();
	}
}

void GameObj::step() {
	const dReal* p;
	
	// Load position, rotation, and velocity from ODE if there are dynamics for this GameObj
	if (body != 0) {
		p = dBodyGetPosition(body);
		pos.x = *p; ++p;
		pos.y = *p; ++p;
		pos.z = *p;
		
		p = dBodyGetRotation(body);
		rot[0] = *p; ++p;
		rot[3] = *p; ++p;
		rot[6] = *p; ++p;
		rot[1] = *p; ++p;
		rot[4] = *p; ++p;
		rot[7] = *p; ++p;
		rot[2] = *p; ++p;
		rot[5] = *p; ++p;
		rot[8] = *p;
		
		p = dBodyGetLinearVel(body);
		vel.x = *p; ++p;
		vel.y = *p; ++p;
		vel.z = *p;
	}
	
	step_impl();
	
	// Apply damping
	if (body != 0) {
		GLint i;
		dReal x, y, z;
		dVector3 v;
		
		p = dBodyGetLinearVel(body);
		x = *p; ++p;
		y = *p; ++p;
		z = *p;
		dBodyVectorFromWorld(body, x, y, z, v);
		for (i = 0; i < 3; ++i) {
			v[i] *= -vel_damp_coef[i]/MAX_FPS;
		}
		dBodyAddRelForce(body, v[0], v[1], v[2]);
		
		p = dBodyGetAngularVel(body);
		x = *p; ++p;
		y = *p; ++p;
		z = *p;
		dBodyVectorFromWorld(body, x, y, z, v);
		for (i = 0; i < 3; ++i) {
			v[i] *= -ang_damp_coef[i]/MAX_FPS;
		}
		dBodyAddRelTorque(body, v[0], v[1], v[2]);
	}
}

void recursive_geom_set_body(dGeomID geom, dBodyID body) {
	if (dGeomIsSpace(geom)) {
		dSpaceID s = dSpaceID(geom);
		GLint end = dSpaceGetNumGeoms(s);
		for (int i = 0; i < end; ++i) {
			recursive_geom_set_body(dSpaceGetGeom(s, i), body);
		}
	} else {
		dGeomSetBody(geom, body);
	}
}

void GameObj::set_body(dBodyID nbody) {
	if (body != 0) {
		dBodyDestroy(body);
	}
	body = nbody;
		
	if (geom != 0) {
		recursive_geom_set_body(geom, body);
	}
	
	// This will load our current position and velocity into the body
	set_pos(pos);
	set_rot(rot);
}

void GameObj::set_geom(dGeomID ngeom) {
	if (geom != 0) {
		dGeomDestroy(geom);
	}
	geom = ngeom;
	if (body != 0) {
		recursive_geom_set_body(geom, body);
	}
}
