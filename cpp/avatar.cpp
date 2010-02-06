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

// How many seconds it takes to move between the Superman and Upright modes, and how much _stance can therefore change each step
const float RUN_STANCE_ENTRY_TIME = 0.5;
const float RUN_STANCE_STEP_SIZE = 1.0f/(RUN_STANCE_ENTRY_TIME*MAX_FPS);

GOAutoRegistration<AvatarGameObj> avatar_gameobj_reg("Avatar");

void AvatarGameObj::step_impl() {
	// TODO: Consider adding linear and angular velocity caps
	// TODO: Set a maximum total acceleration, then force accel vector to be no longer than that magnitude
	
	const Channel* chn;
	float v;
	float cv;
	
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::TranslateX);
	v = -(chn->get_value())*(MAX_STRAFE/MAX_FPS);
	if (chn->is_on()) {
		dBodyAddRelForce(get_body(), v, 0.0, 0.0);
	}
	
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::TranslateY);
	v = -(chn->get_value())*(MAX_STRAFE/MAX_FPS);
	if (chn->is_on()) {
		dBodyAddRelForce(get_body(), 0.0, v, 0.0);
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
	
	// Changing mode between Superman and Upright
	chn = &Input::get_button_ch(ORSave::ButtonBoundAction::RunningStance);
	// Once we are entering or are in running mode, do not assume player wants to leave it unless UprightStance bind is entirely off
	if (chn->is_on() or (chn->is_partially_on() and (_mode == SupermanToUpright or _mode == Upright))) {
		switch (_mode) {
			case Superman:
			case UprightToSuperman:
				_mode = SupermanToUpright;
				break;
			case SupermanToUpright:
				if (similar(_stance, 1.0)) {
					_mode = Upright;
				}
				break;
			default:
				// Do nothing
				break;
		}
	} else {
		switch (_mode) {
			case Upright:
			case SupermanToUpright:
				_mode = UprightToSuperman;
				break;
			case UprightToSuperman:
				if (similar(_stance, 0.0)) {
					_mode = Superman;
				}
				break;
			default:
				// Do nothing
				break;
		}
	}
	
	// Update _stance to match the current mode
	switch (_mode) {
		case Superman:
			_stance = 0.0;
			break;
		case Upright:
		case Attached:
			_stance = 1.0;
			break;
		case SupermanToUpright:
			_stance += RUN_STANCE_STEP_SIZE;
			break;
		case UprightToSuperman:
			_stance -= RUN_STANCE_STEP_SIZE;
			break;
	}
	if (_stance > 1.0) { _stance = 1.0; } else if (_stance < 0.0) { _stance = 0.0; }
	
	// Update the geom's offset to match _stance
	dMatrix3 grot;
	dRFromAxisAndAngle(grot, 1, 0, 0, _stance*M_PI_2);
	dGeomSetOffsetRotation(get_geom(), grot);
}

void AvatarGameObj::near_draw_impl() {
	glRotatef(-_stance*90, 1, 0, 0);
	_anim_fly_to_prerun->draw();
}

AvatarGameObj::AvatarGameObj(const ORE1::ObjType& obj) :
	GameObj(obj),
	_anim_fly_to_prerun(MeshAnimation::load("action-LIBAvatar-Run"))
{
	// TODO Load this information from the ORE mission description
	set_body(Sim::gen_sphere_body(80, 0.5));
	set_geom(dCreateCCylinder(Sim::get_dyn_space(), 0.25, 2.0));
	
	// TODO Maybe some missions start off in Attached mode?
	_mode = Superman;
	_stance = 0.0;
}