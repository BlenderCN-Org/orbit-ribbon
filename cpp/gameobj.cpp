/*
gameobj.cpp: Implementation of the GameObj class
GameObj is the base class for in-game objects, and handles ODE bodies and geoms.

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
#include <boost/array.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <cctype>

#include "autoxsd/orepkgdesc.h"
#include "constants.h"
#include "debug.h"
#include "gameobj.h"
#include "geometry.h"
#include "gloo.h"

// Default coefficients for linear and angular damping on new GameObjs
const float DEFAULT_VEL_DAMP_COEF = 0.15;
const float DEFAULT_ANG_DAMP_COEF = 0.15;

GameObj::GameObj(const ORE1::ObjType& obj) :
	_pos(Point(obj.pos()[0], obj.pos()[1], obj.pos()[2])),
	_vel(Vector()),
	_body(0),
	_geom(0)
{
	std::copy(obj.rot().begin(), obj.rot().end(), _rot.begin());
	
	_vel_damp_coef[0] = DEFAULT_VEL_DAMP_COEF;
	_vel_damp_coef[1] = DEFAULT_VEL_DAMP_COEF;
	_vel_damp_coef[2] = DEFAULT_VEL_DAMP_COEF;
	
	_ang_damp_coef[0] = DEFAULT_ANG_DAMP_COEF;
	_ang_damp_coef[1] = DEFAULT_ANG_DAMP_COEF;
	_ang_damp_coef[2] = DEFAULT_ANG_DAMP_COEF;
}

GameObj::~GameObj() {
	set_body(0);
	set_geom(0);
}

void recursive_geom_set_pos(dGeomID g, const Point& pos) {
	if (dGeomIsSpace(g)) {
		dSpaceID s = dSpaceID(g);
		GLint end = dSpaceGetNumGeoms(s);
		for (int i = 0; i < end; ++i) {
			recursive_geom_set_pos(dSpaceGetGeom(s, i),pos);
		}
	} else {
		dGeomSetPosition(g, pos.x, pos.y, pos.z);
	}
}

void GameObj::set_pos(const Point& pos) {
	_pos = pos;
	
	if (_body != 0) {
		dBodySetPosition(_body, pos.x, pos.y, pos.z);
	} else if (_geom != 0) {
		recursive_geom_set_pos(_geom, pos);
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

void GameObj::set_rot(const boost::array<GLfloat, 9>& rot) {
	_rot = rot;
	
	if (_body != 0 || _geom != 0) {
		// Convert column-major 3x3 to row-major 3x4
		dMatrix3 matr;
		matr[0]  = rot[0]; matr[1]  = rot[3]; matr[2]  = rot[6]; matr[3]  = 0;
		matr[4]  = rot[1]; matr[5]  = rot[4]; matr[6]  = rot[7]; matr[7]  = 0;
		matr[8]  = rot[2]; matr[9]  = rot[5]; matr[10] = rot[8]; matr[11] = 0;
		if (_body != 0) {
			dBodySetRotation(_body, matr);
		} else if (_geom != 0) {
			recursive_geom_set_rot(_geom, matr);
		}
	}
}

std::string GameObj::to_str() const {
	return (boost::format("(P:%s) (R:%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f)")
		% get_pos().to_str()
		% _rot[0]
		% _rot[1]
		% _rot[2]
		% _rot[3]
		% _rot[4]
		% _rot[5]
		% _rot[6]
		% _rot[7]
		% _rot[8]
	).str();
}

void GameObj::draw(bool near) {
	GLOOPushedMatrix pm;
	
	glTranslatef(_pos.x, _pos.y, _pos.z);
	GLfloat rotMatr[16] = {  // Pad out with 4th row and column for OpenGL
		_rot[0], _rot[1], _rot[2], 0,
		_rot[3], _rot[4], _rot[5], 0,
		_rot[6], _rot[7], _rot[8], 0,
		     0,      0,      0,    1
	};
	glMultMatrixf(rotMatr);
	
	if (near) {
		near_draw_impl();
	} else {
		far_draw_impl();
	}
}

void GameObj::step() {
	const dReal* p;
	
	// Load position, rotation, and velocity from ODE if there are dynamics for this GameObj
	if (_body != 0) {
		const dReal* p;
		
		p = dBodyGetPosition(_body);
		_pos.x = p[0];
		_pos.y = p[1];
		_pos.z = p[2];
		
		// For some reason, dMatrix3 has 12 elements, not 9
		// We have to watch out for that here in addition to flipping between column-major and row-major
		p = dBodyGetRotation(_body);
		_rot[0] = p[0]; _rot[1] = p[4]; _rot[2] = p[8];
		_rot[3] = p[1]; _rot[4] = p[5]; _rot[5] = p[9];
		_rot[6] = p[2]; _rot[7] = p[6]; _rot[8] = p[10];
		
		p = dBodyGetLinearVel(_body);
		_vel.x = p[0];
		_vel.y = p[1];
		_vel.z = p[2];
	}
	
	step_impl();
	
	// Apply damping
	if (_body != 0) {
		GLint i;
		dReal x, y, z;
		dVector3 v;
		
		p = dBodyGetLinearVel(_body);
		x = *p; ++p;
		y = *p; ++p;
		z = *p;
		dBodyVectorFromWorld(_body, x, y, z, v);
		for (i = 0; i < 3; ++i) {
			v[i] *= -_vel_damp_coef[i]/MAX_FPS;
		}
		dBodyAddRelForce(_body, v[0], v[1], v[2]);
		
		p = dBodyGetAngularVel(_body);
		x = *p; ++p;
		y = *p; ++p;
		z = *p;
		dBodyVectorFromWorld(_body, x, y, z, v);
		for (i = 0; i < 3; ++i) {
			v[i] *= -_ang_damp_coef[i]/MAX_FPS;
		}
		dBodyAddRelTorque(_body, v[0], v[1], v[2]);
	}
}

Point GameObj::get_rel_point_pos(const Point& p) {
	if (_body == 0) {
		throw GameException("Attempted get_rel_point_pos on a GameObj without an ODE body");
	}
	
	dVector3 res;
	dBodyGetRelPointPos(_body, p.x, p.y, p.z, res);
	return Point(res[0], res[1], res[2]);
}

Vector GameObj::vector_to_world(const Vector& v) {
	if (_body == 0) {
		throw GameException("Attempted vector_to_world on a GameObj without an ODE body");
	}
	
	dVector3 res;
	dBodyVectorToWorld(_body, v.x, v.y, v.z, res);
	return Vector(res[0], res[1], res[2]);
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

void GameObj::set_body(dBodyID body) {
	if (_body != 0) {
		dBodyDestroy(_body);
	}
	_body = body;
		
	if (_geom != 0) {
		recursive_geom_set_body(_geom, _body);
	}
	
	// This will load our current position and velocity into the body
	set_pos(_pos);
	set_rot(_rot);
}

void GameObj::set_geom(dGeomID geom) {
	if (_geom != 0) {
		dGeomDestroy(_geom);
	}
	_geom = geom;
	if (_body != 0) {
		recursive_geom_set_body(_geom, _body);
	}
}

bool GOFactoryRegistry::_initialized;
GOFactoryRegistry::_Factories* GOFactoryRegistry::_factories;

boost::shared_ptr<GameObj> GOFactoryRegistry::create(const ORE1::ObjType& obj) {
	static const boost::regex e("LIB(.+?)(?:\\.\\d+)?");
	
	init();
	boost::smatch m;
	if (boost::regex_match(obj.meshName(), m, e)) {
		std::map<std::string, boost::shared_ptr<GOFactory> >::iterator i = _factories->_factory_map.find(m[1]);
		if (i != _factories->_factory_map.end()) {
			return i->second->create(obj);
		}
	}
	if (!_factories->_default_factory) {
		throw GameException("No GameObj default factory registered!");
	}
	return _factories->_default_factory->create(obj);
}