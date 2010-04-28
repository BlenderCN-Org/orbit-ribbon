/*
avatar.cpp: Implementation of the Avatar class
Avatar is a GameObject representing the player character

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

#include <boost/lexical_cast.hpp>
#include <ode/ode.h>
#include <cmath>
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/gl.h>

#include "autoxsd/orepkgdesc.h"
#include "avatar.h"
#include "constants.h"
#include "debug.h"
#include "geometry.h"
#include "globals.h"
#include "input.h"
#include "mesh.h"
#include "saving.h"
#include "sim.h"

// Maximum amount of Newtons per second applied by various maneuvers
const float MAX_STRAFE = 15000.0;
const float MAX_ACCEL = 15000.0;
const float MAX_TURN = 1000.0;
const float MAX_ROLL = 800.0;

// Counter-turn and counter-roll coefficients
// These act like damping coefficients, but are only turned on when the axis's controls are released
const float CTURN_COEF = 700.0;
const float CROLL_COEF = 700.0;

// How many seconds it takes to move between 0.0 and 1.0 uprightness, and therefore how much _uprightness can change each step
const float UPRIGHTNESS_ENTRY_TIME = 0.5;
const float UPRIGHTNESS_STEP_DIFF = 1.0f/(UPRIGHTNESS_ENTRY_TIME*MAX_FPS);

// Detach grace period is a brief time after an attachment where collisions near the feet of the avatar don't cause contact joints
const float DETACH_GRACE_PERIOD_TIME = 0.05;
const float DETACH_GRACE_PERIOD_TIME_STEPS = DETACH_GRACE_PERIOD_TIME*MAX_FPS;
const float DETACH_GRACE_PERIOD_RADIUS = 0.1;
const float DETACH_GRACE_PERIOD_RADIUS_SQ = DETACH_GRACE_PERIOD_RADIUS * DETACH_GRACE_PERIOD_RADIUS;

GOAutoRegistration<AvatarGameObj> avatar_gameobj_reg("Avatar");

void AvatarGameObj::update_geom_offsets() {
	// Update the orientation of our physical geom to match _uprightness
	dMatrix3 grot;
	dRFromAxisAndAngle(grot, 1, 0, 0, _uprightness*M_PI_2);
	dGeomSetOffsetRotation(get_entity().get_geom("physical"), grot);
	
	// Update the position of the sticky_attach geom to match _uprightness
	dGeomSetOffsetPosition(get_entity().get_geom("sticky_attach"), 0, -sin(_uprightness*M_PI_2) + RUNNING_MAX_DELTA_Y_POS, 0);
}

bool AvatarGameObj::check_attachment(float yp_delta, const Vector& sn) {
	// TODO Consider using two contact points, one for each foot, then averaging out the plane normal
	// TODO Check if the other object is runnable/attachable
	// TODO Shouldn't just assume that first contact of first collision is the best one to use
	// TODO Maybe allow a special "walking" or "crawling" attachment when relative velocity to surface is very low
	// TODO Don't attach when relative velocity is non-trivial and too different from facing direction
	// TODO Maybe treat linear velocity projected onto XZ as another one of these limited variables, to simulate friction
	
	// If multiple potential collisions occurred this frame, choose the one with the least ypos delta
	// Hopefully this will stop the sticky attachment ray from trying to attach us to a more distant geom beneath the floor
	if (_attached_this_frame) {
		if (std::fabs(yp_delta) > std::fabs(_ypos_delta)) {
			return false;
		}
	}
	
	_ypos_delta = yp_delta;
	if (std::fabs(_ypos_delta) > RUNNING_MAX_DELTA_Y_POS) {
		return false;
	}
	
	dBodyID body = get_entity().get_id();
	bool ret = false;
	Vector sn_rel = vector_from_world(sn);
	Vector lvel_rel = vector_from_world(Vector(dBodyGetLinearVel(body)));
	Vector avel_rel = vector_from_world(Vector(dBodyGetAngularVel(body)));
	
	// Offsets to various values that we'd like to apply for optimal running state
	_sn = sn;
	_xrot_delta = -std::asin(sn_rel.z); // Rotate about x axis to reduce z to 0
	_zrot_delta = -std::asin(sn_rel.x); // Rotate about z axis to reduce x to 0
	_ylvel_delta = -lvel_rel.y;
	_xavel_delta = -avel_rel.x;
	_zavel_delta = -avel_rel.z;
	
	// If the difference between current and ideal state is not too severe, attach to surface
	if (
		std::fabs(_xrot_delta) <= RUNNING_MAX_DELTA_X_ROT &&
		std::fabs(_zrot_delta) <= RUNNING_MAX_DELTA_Z_ROT &&
		std::fabs(_ylvel_delta) <= RUNNING_MAX_DELTA_Y_LVEL &&
		std::fabs(_xavel_delta) <= RUNNING_MAX_DELTA_X_AVEL &&
		std::fabs(_zavel_delta) <= RUNNING_MAX_DELTA_Z_AVEL
	) {
		ret = true;
	} else {
		/*
		Debug::debug_msg(std::string("ATTACH FAILURE") +
			" YPOS:" + boost::lexical_cast<std::string>(_ypos_delta) +
			" XROT:" + boost::lexical_cast<std::string>(_xrot_delta) +
			" ZROT:" + boost::lexical_cast<std::string>(_zrot_delta) +
			" YLVEL:" + boost::lexical_cast<std::string>(_ylvel_delta) +
			" XAVEL:" + boost::lexical_cast<std::string>(_xavel_delta) +
			" ZAVEL:" + boost::lexical_cast<std::string>(_zavel_delta)
		);
		*/
	}
	
	if (ret) {
		_attached_this_frame = true;
	}
	return ret;
}

bool AvatarGameObj::AvatarContactHandler::handle_collision(
	float t __attribute__ ((unused)),
	dGeomID o __attribute__ ((unused)),
	const dContactGeom* c,
	unsigned int c_len __attribute__ ((unused))
)  {
	float ypd = c[0].depth;
	Vector sn(c[0].normal);
	const GLOOBufferedMesh* mesh = GLOOBufferedMesh::get_mesh_from_geom(c[0].g2);
	if (mesh != 0) {
		sn = mesh->get_interpolated_normal(c[0].g2, Point(c[0].pos), c[0].side2);
	}
	
	if (_avatar->check_attachment(ypd, sn)) {
		_avatar->_run_coll_steptime = Globals::total_steps;
		return false;
	} else {
		Point feet_center(_avatar->get_pos() - _avatar->vector_to_world(Vector(0, _avatar->_height/2, 0)));
		Point coll = Point(c[0].pos);
		if (
			_avatar->get_last_run_coll_age() <= DETACH_GRACE_PERIOD_TIME_STEPS &&
			feet_center.sq_dist_to(coll) <= DETACH_GRACE_PERIOD_RADIUS_SQ
		) {
			return false;
		} else {
			_avatar->_norm_coll_steptime = Globals::total_steps;
			return true;
		}
	}
}

bool AvatarGameObj::StickyAttachmentContactHandler::handle_collision(
	float t __attribute__ ((unused)),
	dGeomID o __attribute__ ((unused)),
	const dContactGeom* c,
	unsigned int c_len  __attribute__ ((unused))
)  {
	float ypd = -c[0].depth + RUNNING_MAX_DELTA_Y_POS;
	Vector sn(c[0].normal);
	const GLOOBufferedMesh* mesh = GLOOBufferedMesh::get_mesh_from_geom(c[0].g2);
	if (mesh != 0) {
		sn = mesh->get_interpolated_normal(c[0].g2, Point(c[0].pos), c[0].side2);
	}
	
	_avatar->check_attachment(ypd, sn);
	
	// This geom is never used to create contact joints
	return false;
}

void AvatarGameObj::step_impl() {
	dBodyID body = get_entity().get_id();
	
	const Channel* chn;
	
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::TranslateX);
	if (chn->is_on()) {
		float v = (chn->get_value())*(MAX_STRAFE/MAX_FPS);
		dBodyAddRelForce(body, -v, 0.0, 0.0);
	}
	
	bool pushing_up = false;
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::TranslateY);
	if (chn->is_on()) {
		float v = (chn->get_value())*(MAX_STRAFE/MAX_FPS);
		if (Saving::get().config().invertTranslateY().get()) {
			v = -v;
		}
		dBodyAddRelForce(body, 0.0, -v, 0.0);
		if (v < 0) {
			pushing_up = true;
		}
	}
	
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::TranslateZ);
	if (chn->is_on()) {
		float v = (chn->get_value())*(MAX_ACCEL/MAX_FPS);
		dBodyAddRelForce(body, 0.0, 0.0, v);
	}
	
	const dReal* avel = dBodyGetAngularVel(body);
	dVector3 rel_avel;
	dBodyVectorFromWorld(body, avel[0], avel[1], avel[2], rel_avel);
	
	// X-turn and x-counterturn
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::RotateY);
	if (chn->is_on()) {
		float v = -(chn->get_value())*(MAX_TURN/MAX_FPS);
		dBodyAddRelTorque(body, 0.0, v, 0.0);
	} else {
		float cv = rel_avel[1]*-CTURN_COEF/MAX_FPS;
		dBodyAddRelTorque(body, 0.0, cv, 0.0);
	}
	
	// Y-turn and y-counterturn
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::RotateX);
	if (chn->is_on()) {
		float v = (chn->get_value())*(MAX_TURN/MAX_FPS);
		if (Saving::get().config().invertRotateY().get()) {
			v = -v;
		}
		dBodyAddRelTorque(body, v, 0.0, 0.0);
	} else {
		float cv = rel_avel[0]*-CTURN_COEF/MAX_FPS;
		dBodyAddRelTorque(body, cv, 0.0, 0.0);
	}
	
	// Roll and counter-roll
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::RotateZ);
	if (chn->is_on()) {
		float v = (chn->get_value())*(MAX_ROLL/MAX_FPS);
		dBodyAddRelTorque(body, 0.0, 0.0, v);
	} else {
		float cv = rel_avel[2]*(-CROLL_COEF/MAX_FPS);
		dBodyAddRelTorque(body, 0.0, 0.0, cv);
	}
	
	// Changing stance between superman-style and upright
	if (_attached) {
		_uprightness += UPRIGHTNESS_STEP_DIFF;
	} else {
		_uprightness -= UPRIGHTNESS_STEP_DIFF;
	}
	if (_uprightness > 1.0) { _uprightness = 1.0; } else if (_uprightness < 0.0) { _uprightness = 0.0; }
	
	update_geom_offsets();
	
	_attached = _attached_this_frame;
	
	// If we are attached, work to keep ourselves ideally oriented to the attachment surface
	if (_attached) {
		Vector sn_rel = vector_from_world(_sn);
		Vector lvel = Vector(dBodyGetLinearVel(body));
		Vector lvel_rel = vector_from_world(lvel);
		Vector avel = Vector(dBodyGetAngularVel(body));
		Vector avel_rel = vector_from_world(avel);
		
		// Apply as much of each delta as we can
		
		// X and Z orientation delta
		// TODO Maybe should translate body so that the contact point stays in the same spot through rotation
		float a = limit_abs(_zrot_delta, RUNNING_ADJ_RATE_Z_ROT/MAX_FPS);
		Vector body_x(vector_to_world(Vector(cos(a), sin(a), 0)));
		a = limit_abs(-_xrot_delta, RUNNING_ADJ_RATE_X_ROT/MAX_FPS);
		Vector body_y(vector_to_world(Vector(0, cos(a), sin(a))));
		dMatrix3 matr;
		dRFrom2Axes(matr, body_x.x, body_x.y, body_x.z, body_y.x, body_y.y, body_y.z);
		dBodySetRotation(body, matr);
		
		// Y position delta
		// If the user is pushing up, set the target point high above the ground so we escape sticky attachment
		set_pos(get_pos() + _sn*limit_abs(_ypos_delta + (pushing_up ? RUNNING_MAX_DELTA_Y_POS*2 : 0), RUNNING_ADJ_RATE_Y_POS/MAX_FPS));
		
		// Y linear velocity delta
		lvel_rel.y += limit_abs(_ylvel_delta, RUNNING_ADJ_RATE_Y_LVEL/MAX_FPS);
		lvel = vector_to_world(lvel_rel);
		dBodySetLinearVel(body, lvel.x, lvel.y, lvel.z);
		
		// X and Z angular velocity delta
		avel_rel.x += limit_abs(_xavel_delta, RUNNING_ADJ_RATE_X_AVEL/MAX_FPS);
		avel_rel.z += limit_abs(_zavel_delta, RUNNING_ADJ_RATE_Z_AVEL/MAX_FPS);
		avel = vector_to_world(avel_rel);
		dBodySetAngularVel(body, avel.x, avel.y, avel.z);
	}
	
	if (_attached_this_frame) {
		_attached_this_frame = false;
		dGeomEnable(get_entity().get_geom("sticky_attach"));
	} else {
		dGeomDisable(get_entity().get_geom("sticky_attach"));
	}
}

void AvatarGameObj::near_draw_impl() {
	glRotatef(-_uprightness*90, 1, 0, 0);
	_anim_fly_to_prerun->draw();	
}

AvatarGameObj::AvatarGameObj(const ORE1::ObjType& obj) :
	GameObj(obj, Sim::gen_sphere_body(80, 0.5)), // TODO Load mass information from the ORE mission description
	_xrot_delta(0.0),
	_zrot_delta(0.0),
	_ypos_delta(0.0),
	_ylvel_delta(0.0),
	_xavel_delta(0.0),
	_zavel_delta(0.0),
	_norm_coll_steptime(0),
	_run_coll_steptime(0),
	_anim_fly_to_prerun(MeshAnimation::load("action-LIBAvatar-Run")),
	_attached(false),
	_attached_this_frame(false)
{
	// TODO Load volume information from the ORE mission description
	_height = 2.25;
	_coll_rad = 0.25;
	
	// Set up a geom for detecting regular collisions
	get_entity().set_geom(
		"physical",
		dCreateCapsule(Sim::get_dyn_space(), _coll_rad, _height - 2*_coll_rad),
		std::auto_ptr<CollisionHandler>(new AvatarContactHandler(this))
	);
	
	// TODO Maybe some missions start off in upright mode?
	_uprightness = 0.0;
	
	// Set up a geom at our feet to detect when we can run on a surface
	get_entity().set_geom(
		"sticky_attach",
		dCreateRay(Sim::get_dyn_space(), RUNNING_MAX_DELTA_Y_POS*2),
		std::auto_ptr<CollisionHandler>(new StickyAttachmentContactHandler(this))
	);
	dQuaternion rdq;
	dQFromAxisAndAngle(rdq, 1, 0, 0, M_PI_2);
	dGeomSetOffsetQuaternion(get_entity().get_geom("sticky_attach"), rdq);
	dGeomDisable(get_entity().get_geom("sticky_attach"));
	
	update_geom_offsets();
}

unsigned int AvatarGameObj::get_last_norm_coll_age() {
	return _norm_coll_steptime == 0 ? 100000 : Globals::total_steps - _norm_coll_steptime;
}

unsigned int AvatarGameObj::get_last_run_coll_age() {
	return _run_coll_steptime == 0 ? 100000 : Globals::total_steps - _run_coll_steptime;
}