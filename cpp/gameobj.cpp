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
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <cctype>
#include <vector>

#include "autoxsd/orepkgdesc.h"
#include "constants.h"
#include "debug.h"
#include "except.h"
#include "gameobj.h"
#include "geometry.h"
#include "gloo.h"

// Default coefficients for linear and angular damping on new GameObjs
const float DEFAULT_VEL_DAMP_COEF = 0.15;
const float DEFAULT_ANG_DAMP_COEF = 0.15;

GameObj::GameObj(const ORE1::ObjType& obj, std::auto_ptr<OdeEntity> entity) :
	_pos(Point(obj.pos()[0], obj.pos()[1], obj.pos()[2])),
	_vel(Vector()),
	_entity(entity)
{
	if (_entity.get() == 0) {
		throw GameException("Cannot pass null OdeEntity pointer to GameObj. You probably want Sim::gen_empty_body()");
	}
	
	std::copy(obj.rot().begin(), obj.rot().end(), _rot.begin());
	
	_entity->set_pos(_pos);
	_entity->set_rot(_rot);
	_entity->set_gameobj(this);
	
	_vel_damp_coef[0] = DEFAULT_VEL_DAMP_COEF;
	_vel_damp_coef[1] = DEFAULT_VEL_DAMP_COEF;
	_vel_damp_coef[2] = DEFAULT_VEL_DAMP_COEF;
	
	_ang_damp_coef[0] = DEFAULT_ANG_DAMP_COEF;
	_ang_damp_coef[1] = DEFAULT_ANG_DAMP_COEF;
	_ang_damp_coef[2] = DEFAULT_ANG_DAMP_COEF;
}

void GameObj::set_pos(const Point& pos) {
	_pos = pos;
	_entity->set_pos(pos);
}

void GameObj::set_rot(const boost::array<float, 9>& rot) {
	_rot = rot;
	_entity->set_rot(rot);
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
	float rotMatr[16] = {  // Pad out with 4th row and column for OpenGL
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
	dBodyID b = 0;
	if (_entity->has_id()) {
		b = _entity->get_id();
	}
	
	// Load position, rotation, and velocity from ODE if there are dynamics for this GameObj
	if (b != 0) {
		const dReal* p;
		
		p = dBodyGetPosition(b);
		_pos.x = p[0];
		_pos.y = p[1];
		_pos.z = p[2];
		
		// For some reason, dMatrix3 has 12 elements, not 9, where the extra column has no useful information for us
		// We have to watch out for that here in addition to flipping between column-major and row-major
		p = dBodyGetRotation(b);
		_rot[0] = p[0]; _rot[1] = p[4]; _rot[2] = p[8];
		_rot[3] = p[1]; _rot[4] = p[5]; _rot[5] = p[9];
		_rot[6] = p[2]; _rot[7] = p[6]; _rot[8] = p[10];
		
		p = dBodyGetLinearVel(b);
		_vel.x = p[0];
		_vel.y = p[1];
		_vel.z = p[2];
	}
	
	step_impl();
	
	// Apply damping
	if (b != 0) {
		GLint i;
		dReal x, y, z;
		dVector3 v;
		
		p = dBodyGetLinearVel(b);
		x = *p; ++p;
		y = *p; ++p;
		z = *p;
		dBodyVectorFromWorld(b, x, y, z, v);
		for (i = 0; i < 3; ++i) {
			v[i] *= -_vel_damp_coef[i]/MAX_FPS;
		}
		dBodyAddRelForce(b, v[0], v[1], v[2]);
		
		p = dBodyGetAngularVel(b);
		x = *p; ++p;
		y = *p; ++p;
		z = *p;
		dBodyVectorFromWorld(b, x, y, z, v);
		for (i = 0; i < 3; ++i) {
			v[i] *= -_ang_damp_coef[i]/MAX_FPS;
		}
		dBodyAddRelTorque(b, v[0], v[1], v[2]);
	}
}

Point GameObj::get_rel_point_pos(const Point& p) const {
	dVector3 res;
	dBodyGetRelPointPos(_entity->get_id(), p.x, p.y, p.z, res);
	return Point(res);
}

Point GameObj::get_pos_rel_point(const Point& p) const {
	dVector3 res;
	dBodyGetPosRelPoint(_entity->get_id(), p.x, p.y, p.z, res);
	return Point(res);
}

Vector GameObj::vector_to_world(const Vector& v) const {
	dVector3 res;
	dBodyVectorToWorld(_entity->get_id(), v.x, v.y, v.z, res);
	return Vector(res);
}

Vector GameObj::vector_from_world(const Vector& v) const {
	dVector3 res;
	dBodyVectorFromWorld(_entity->get_id(), v.x, v.y, v.z, res);
	return Vector(res);
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