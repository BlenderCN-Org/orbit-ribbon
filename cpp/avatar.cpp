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

#include "autoxsd/orepkgdesc.h"
#include "avatar.h"
#include "constants.h"
#include "debug.h"
#include "geometry.h"
#include "globals.h"
#include "input.h"
#include "mesh.h"
#include "sim.h"

// Maximum amount of Newtons per second applied by various maneuvers
const float MAX_STRAFE = 5000.0;
const float MAX_ACCEL = 15000.0;
const float MAX_TURN = 1000.0;
const float MAX_ROLL = 800.0;

// Counter-turn and counter-roll coefficients
// These act like damping coefficients, but are only turned on when the axis's controls are released
const float CTURN_COEF = 700.0;
const float CROLL_COEF = 700.0;

// How many seconds it takes to move between 0.0 and 1.0 uprightness, and therefore how much _uprightness change each step
const float UPRIGHTNESS_ENTRY_TIME = 0.5;
const float UPRIGHTNESS_STEP_DIFF = 1.0f/(UPRIGHTNESS_ENTRY_TIME*MAX_FPS);

// Maximum absolute x (roll offset) and z (pitch offset) values in running_coll contact vector to begin running
// The larger these numbers, the worse the player's approach to a running surface can be
const float RUNNING_MAX_X = 0.2;
const float RUNNING_MAX_Z = 0.4;

GOAutoRegistration<AvatarGameObj> avatar_gameobj_reg("Avatar");

void AvatarGameObj::RunningCollisionHandler::handle_collision(dGeomID other, const GameObj* other_gameobj, const dContactGeom* contacts, unsigned int contacts_len __attribute__ ((unused))) {
	// TODO Check if the other object is runnable/attachable
	// TODO Should I not just assume that the first contact is the most appropriate one to use?
	_normal = Vector(contacts[0].normal[0], contacts[0].normal[1], contacts[0].normal[2]);
	_depth = contacts[0].depth;
	_dirty = true;
}

bool AvatarGameObj::RunningCollisionHandler::check_dirty() {
	if (_dirty) {
		_dirty = false; return true;
	} else {
		return false;
	}
}

void AvatarGameObj::step_impl() {
	// TODO: Consider adding linear and angular velocity caps
	// TODO: Set a maximum total acceleration, then force accel vector to be no longer than that magnitude
	
	const Channel* chn;
	float v;
	float cv;
	
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::TranslateX);
	v = (chn->get_value())*(MAX_STRAFE/MAX_FPS);
	if (chn->is_on()) {
		dBodyAddRelForce(get_body(), -v, 0.0, 0.0);
	}
	
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::TranslateY);
	v = (chn->get_value())*(MAX_STRAFE/MAX_FPS);
	if (chn->is_on()) {
		dBodyAddRelForce(get_body(), 0.0, -v, 0.0);
	}
	
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::TranslateZ);
	v = (chn->get_value())*(MAX_ACCEL/MAX_FPS);
	if (chn->is_on()) {
		dBodyAddRelForce(get_body(), 0.0, 0.0, v);
	}
	
	const dReal* avel = dBodyGetAngularVel(get_body());
	dVector3 rel_avel;
	dBodyVectorFromWorld(get_body(), avel[0], avel[1], avel[2], rel_avel);
	
	// X-turn and x-counterturn
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::RotateY);
	v = -(chn->get_value())*(MAX_TURN/MAX_FPS);
	cv = rel_avel[1]*-CTURN_COEF/MAX_FPS;
	if (chn->is_on()) {
		dBodyAddRelTorque(get_body(), 0.0, v, 0.0);
	} else {
		dBodyAddRelTorque(get_body(), 0.0, cv, 0.0);
	}
	
	// Y-turn and y-counterturn
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::RotateX);
	v = (chn->get_value())*(MAX_TURN/MAX_FPS);
	cv = rel_avel[0]*-CTURN_COEF/MAX_FPS;
	if (chn->is_on()) {
		dBodyAddRelTorque(get_body(), v, 0.0, 0.0);
	} else {
		dBodyAddRelTorque(get_body(), cv, 0.0, 0.0);
	}
	
	// Roll and counter-roll
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::RotateZ);
	v = (chn->get_value())*(MAX_ROLL/MAX_FPS);
	cv = rel_avel[2]*(-CROLL_COEF/MAX_FPS);
	if (chn->is_on()) {
		dBodyAddRelTorque(get_body(), 0.0, 0.0, v);
	} else {
		dBodyAddRelTorque(get_body(), 0.0, 0.0, cv);
	}
	
	// Changing stance between superman-style and upright
	chn = &Input::get_button_ch(ORSave::ButtonBoundAction::UprightStance);
	// Once we are entering or are in upright mode, do not assume player wants to leave it unless UprightStance bind is entirely off
	if (chn->is_on() or (chn->is_partially_on() and !similar(_uprightness, 0.0))) {
		_uprightness += UPRIGHTNESS_STEP_DIFF;
	} else {
		_uprightness -= UPRIGHTNESS_STEP_DIFF;
	}
	if (_uprightness > 1.0) { _uprightness = 1.0; } else if (_uprightness < 0.0) { _uprightness = 0.0; }
	
	// Update the offsets of our geoms to match _stance
	dGeomSetOffsetPosition(get_geom("run"), 0, std::sin(-_uprightness*M_PI_2)*1.25, 0); // FIXME Figure out avatar height dynamically
	dMatrix3 grot;
	dRFromAxisAndAngle(grot, 1, 0, 0, _uprightness*M_PI_2);
	dGeomSetOffsetRotation(get_geom("physical"), grot);
	
	// Check if we are contacting a surface in a way that allows attaching
	if (_run_coll_handler.check_dirty()) {
		Vector c = vector_from_world(_run_coll_handler.get_contact_normal());
		float d = _run_coll_handler.get_depth();
		if (c.y > 0 && std::fabs(c.x) < RUNNING_MAX_X && std::fabs(c.z) < RUNNING_MAX_Z) {
			// Attach feet to this surface, and stay aligned to it
			//dBodyAddRelForce(get_body(), 0.0, -200.0, 0.0);
			dBodyAddRelTorque(get_body(), c.z*1000, 0.0, 0.0);
		}
	}
}

void AvatarGameObj::near_draw_impl() {
	glRotatef(-_uprightness*90, 1, 0, 0);
	_anim_fly_to_prerun->draw();
}

AvatarGameObj::AvatarGameObj(const ORE1::ObjType& obj) :
	GameObj(obj),
	_anim_fly_to_prerun(MeshAnimation::load("action-LIBAvatar-Run")),
	_run_coll_handler(this)
{
	// TODO Load mass and volume information from the ORE mission description
	set_body(Sim::gen_sphere_body(80, 0.5));
	set_geom("physical", dCreateCCylinder(Sim::get_dyn_space(), 0.25, 2.0));
	
	// Set up a geom at our feet to detect when we can run on a surface
	dGeomID run_geom = dCreateSphere(Sim::get_dyn_space(), 0.3);
	dGeomSetData(run_geom, (void*)&_run_coll_handler);
	set_geom("run", run_geom);
	
	// TODO Maybe some missions start off in upright mode?
	_uprightness = 0.0;
}