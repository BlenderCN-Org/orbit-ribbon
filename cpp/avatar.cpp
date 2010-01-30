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

#include <ode/ode.h>

#include "autoxsd/orepkgdesc.h"
#include "avatar.h"
#include "constants.h"
#include "geometry.h"
#include "input.h"
#include "mesh.h"
#include "sim.h"

// Maximum amount of Newtons per second applied by various maneuvers
const float MAX_STRAFE = 5000;
const float MAX_ACCEL = 15000;
const float MAX_TURN = 1000;
const float MAX_ROLL = 800;

// Counter-turn and counter-roll coefficients
// These act like damping coefficients, but are only turned on when the axis's controls are released
const float CTURN_COEF = 700;
const float CROLL_COEF = 700;

GOAutoRegistration<AvatarGameObj> avatar_gameobj_reg("Avatar");

void AvatarGameObj::step_impl() {
	// TODO: Consider adding linear and angular velocity caps
	// TODO: Set a maximum total acceleration, then force accel vector to be no longer than that magnitude
		
	float v;
	
	v = Input::get_axis_ch(ORSave::AxisBoundAction::TranslateX).get_value();
	if (v != 0.0) {
		dBodyAddRelForce(get_body(), -v*(MAX_STRAFE/MAX_FPS), 0.0, 0.0);
	}
	
	v = Input::get_axis_ch(ORSave::AxisBoundAction::TranslateY).get_value();
	if (v != 0.0) {
		dBodyAddRelForce(get_body(), 0.0, -v*(MAX_STRAFE/MAX_FPS), 0.0);
	}
	
	v = Input::get_axis_ch(ORSave::AxisBoundAction::TranslateZ).get_value();
	if (v != 0.0) {
		dBodyAddRelForce(get_body(), 0.0, 0.0, v*(MAX_ACCEL/MAX_FPS));
	}
	
	const dReal* avel = dBodyGetAngularVel(get_body());
	dVector3 rel_avel;
	dBodyVectorFromWorld(get_body(), avel[0], avel[1], avel[2], rel_avel);
	
	// X-turn and x-counterturn
	v = Input::get_axis_ch(ORSave::AxisBoundAction::RotateY).get_value();
	if (v != 0.0) {
		dBodyAddRelTorque(get_body(), 0.0, -v*(MAX_TURN/MAX_FPS), 0.0);
	} else {
		dBodyAddRelTorque(get_body(), 0.0, rel_avel[1]*-CTURN_COEF/MAX_FPS, 0.0);
	}
	
	// Y-turn and y-counterturn
	v = Input::get_axis_ch(ORSave::AxisBoundAction::RotateX).get_value();
	if (v != 0.0) {
		dBodyAddRelTorque(get_body(), v*(MAX_TURN/MAX_FPS), 0.0, 0.0);
	} else {
		dBodyAddRelTorque(get_body(), rel_avel[0]*-CTURN_COEF/MAX_FPS, 0.0, 0.0);
	}
	
	// Roll
	v = Input::get_axis_ch(ORSave::AxisBoundAction::RotateZ).get_value();
	if (v != 0.0) {
		dBodyAddRelTorque(get_body(), 0.0, 0.0, v*(MAX_ROLL/MAX_FPS));
	} else {
		dBodyAddRelTorque(get_body(), 0.0, 0.0, rel_avel[2]*(-CROLL_COEF/MAX_FPS));
	}
}

void AvatarGameObj::near_draw_impl() {
	_anim_fly_to_prerun->draw();
}

AvatarGameObj::AvatarGameObj(const ORE1::ObjType& obj) :
	GameObj(obj),
	_anim_fly_to_prerun(MeshAnimation::load("action-LIBAvatar-Run"))
{
	set_body(Sim::gen_sphere_body(80, 0.5));
}
