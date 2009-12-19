#include <GL/gl.h>
#include <ode/ode.h>

#include "gameobj.h"
#include "geometry.h"

GameObj::GameObj(const Point& npos, const Rotation& nrot) :
	pos(npos),
	rot(nrot),
	vel(Vector()),
	vel_damp_coef(get_default_vel_damp_coef()),
	ang_damp_coef(get_default_ang_damp_coef()),
	body(0),
	geom(0)
{}

GameObj::~GameObj() {
	set_body(0);
	set_geom(0);
}

recursive_geom_set_pos(dGeomID g, const Point& npos) {
	if (dGeomIsSpace(g)) {
		GLint end = dSpaceGetNumGeoms(g);
		for (int i = 0; i < end; ++i) {
			recursive_geom_set_pos(dSpaceGetGeom(g, i), npos);
		}
	} else {
		dGeomSetPosition(g, npos.x, npos.y, npos.z);
	}
}

void GameObj::set_pos(const Point& npos) {
	pos = npos;
	
	if (body != 0) {
		dBodySetPosition(body, point.x, point.y, point.z);
	} else if (geom != 0) {
		recursive_geom_set_pos(geom, point);
	}
}

recursive_geom_set_rot(dGeomID g, const dMatrix3& matr) {
	if (dGeomIsSpace(g)) {
		GLint end = dSpaceGetNumGeoms(g);
		for (int i = 0; i < end; ++i) {
			recursive_geom_set_rot(dSpaceGetGeom(g, i), matr);
		}
	} else {
		dGeomSetRotation(g, matr);
	}
}

void GameObj::set_rot(const Rotation& nrot) {
	rot = nrot;

	dMatrix3 matr;
	dRFromEulerAngles(matr, nrot.x, nrot.y, nrot.z);
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
		// FIXME Convert ODE rotation matrix to Euler rotation
		// FIXME Also, make sure that Euler rotation is what I'm actually using, heh
		
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
			v[i] *= vel_damp_coef/MAXFPS; // FIXME : This is where I left off; can't reach maxfps from here, need a seperate constants header
		}
	}
}

void GameObj::set_body(dBodyID nbody) {
	if (body != 0) {
		dBodyDestroy(body);
	}
	body = nbody;
	dBodySetPosition(body, point.x, point.y, point.z);
	dMatrix3 matr;
	dRFromEulerAngles(matr, rot.x, rot.y, rot.z);
	dBodySetRotation(body, matr);
	if (geom != 0) {
		dGeomSetBody(geom, body);
	}
}

void GameObj::set_geom(dGeomID ngeom) {
	if (geom != 0) {
		dGeomDestroy(geom);
	}
	geom = ngeom;
	if (body != 0) {
		dGeomSetBody(geom, body);
	} else {
		recursive_geom_set_pos(geom, pos);
		dMatrix3 matr;
		dRFromEulerAngles(matr, rot.x, rot.y, rot.z);
		recursive_geom_set_rot(geom, matr);
	}
}
