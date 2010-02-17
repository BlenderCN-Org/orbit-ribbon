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
const float RUNNING_MAX_X = 0.4;
const float RUNNING_MAX_Z = 0.4;

// Radius of the sphere used to detect when feet are near a running surface
const float RUNNING_SPHERE_RAD = 0.25;

// When attached, how deep the running detection geom should stay embedded in the surface
const float RUN_DETECT_TARGET_DEPTH = 0.05;

GOAutoRegistration<AvatarGameObj> avatar_gameobj_reg("Avatar");

void AvatarGameObj::step_impl() {
	// TODO: Consider adding linear and angular velocity caps
	// TODO: Set a maximum total acceleration, then force accel vector to be no longer than that magnitude
	
	dBodyID body = get_entity().get_id();
	
	const Channel* chn;
	float v;
	float cv;
	
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::TranslateX);
	v = (chn->get_value())*(MAX_STRAFE/MAX_FPS);
	if (chn->is_on()) {
		dBodyAddRelForce(body, -v, 0.0, 0.0);
	}
	
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::TranslateY);
	v = (chn->get_value())*(MAX_STRAFE/MAX_FPS);
	if (chn->is_on()) {
		dBodyAddRelForce(body, 0.0, -v, 0.0);
	}
	
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::TranslateZ);
	v = (chn->get_value())*(MAX_ACCEL/MAX_FPS);
	if (chn->is_on()) {
		dBodyAddRelForce(body, 0.0, 0.0, v);
	}
	
	const dReal* avel = dBodyGetAngularVel(body);
	dVector3 rel_avel;
	dBodyVectorFromWorld(body, avel[0], avel[1], avel[2], rel_avel);
	
	// X-turn and x-counterturn
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::RotateY);
	v = -(chn->get_value())*(MAX_TURN/MAX_FPS);
	cv = rel_avel[1]*-CTURN_COEF/MAX_FPS;
	if (chn->is_on()) {
		dBodyAddRelTorque(body, 0.0, v, 0.0);
	} else {
		dBodyAddRelTorque(body, 0.0, cv, 0.0);
	}
	
	// Y-turn and y-counterturn
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::RotateX);
	v = (chn->get_value())*(MAX_TURN/MAX_FPS);
	cv = rel_avel[0]*-CTURN_COEF/MAX_FPS;
	if (chn->is_on()) {
		dBodyAddRelTorque(body, v, 0.0, 0.0);
	} else {
		dBodyAddRelTorque(body, cv, 0.0, 0.0);
	}
	
	// Roll and counter-roll
	chn = &Input::get_axis_ch(ORSave::AxisBoundAction::RotateZ);
	v = (chn->get_value())*(MAX_ROLL/MAX_FPS);
	cv = rel_avel[2]*(-CROLL_COEF/MAX_FPS);
	if (chn->is_on()) {
		dBodyAddRelTorque(body, 0.0, 0.0, v);
	} else {
		dBodyAddRelTorque(body, 0.0, 0.0, cv);
	}
	
	// Changing stance between superman-style and upright
	chn = &Input::get_button_ch(ORSave::ButtonBoundAction::UprightStance);
	// Once we are entering or are in upright mode, do not assume player wants to leave it unless UprightStance bind is entirely off
	if (chn->is_on() or (chn->is_partially_on() and !similar(_uprightness, 0.0))) {
		_uprightness += UPRIGHTNESS_STEP_DIFF;
	} else if (!_attached) {
		_uprightness -= UPRIGHTNESS_STEP_DIFF;
	}
	if (_uprightness > 1.0) { _uprightness = 1.0; } else if (_uprightness < 0.0) { _uprightness = 0.0; }
	
	// Update the offsets of our geoms to match _stance
	dGeomSetOffsetPosition(get_entity().get_geom("run_detect"), 0, std::sin(-_uprightness*M_PI_2)*1.10, 0); // FIXME Figure out avatar height dynamically
	dMatrix3 grot;
	dRFromAxisAndAngle(grot, 1, 0, 0, _uprightness*M_PI_2);
	dGeomSetOffsetRotation(get_entity().get_geom("physical"), grot);
	
	// Check if we are contacting a surface in a way that allows attaching
	RunCollisionTracker* rct = static_cast<RunCollisionTracker*>(get_entity().get_geom_ch("run_detect"));
	_attached = false;
	if (rct->has_collisions()) {
		std::auto_ptr<std::vector<CollisionTracker::Collision> > colls = rct->get_collisions();
		
		// TODO Check if the other object is runnable/attachable
		// TODO Shouldn't just assume that first contact of last collision is the best one to use
		
		Vector c = Vector(
			colls->back().contacts[0].normal[0],
			colls->back().contacts[0].normal[1],
			colls->back().contacts[0].normal[2]
		);
		Vector c_rel = vector_from_world(c);
		float d = colls->back().contacts[0].depth;
		float delta = d - RUN_DETECT_TARGET_DEPTH;
		if (c_rel.y > 0 && std::fabs(c_rel.x) < RUNNING_MAX_X && std::fabs(c_rel.z) < RUNNING_MAX_Z) {
			_run_coll_occurred = true;
			_attached = true;
			
			// Keep us the right distance from the surface
			Point p = get_pos();
			dBodySetPosition(body, p.x + c.x*delta, p.y + c.y*delta, p.z + c.z*delta);
			
			// Keep us oriented perpindicularly to the surface without rotating about the y axis
			// FIXME Translate body so that the contact point stays in the same spot through rotation
			Vector body_x = vector_to_world(Vector(1, 0, 0)); // TODO Project this onto the plane defined by the c vector
			dMatrix3 matr;
			dRFrom2Axes(matr, body_x.x, body_x.y, body_x.z, c.x, c.y, c.z);
			dBodySetRotation(body, matr);
		}
	}
}

void AvatarGameObj::near_draw_impl() {
	glRotatef(-_uprightness*90, 1, 0, 0);
	_anim_fly_to_prerun->draw();
	
	if (Saving::get().config().debugPhysics().get()) {
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		GLUquadric* q = gluNewQuadric();
		
		if (_run_coll_occurred) {
			glColor4f(1.0, 0.0, 0.0, 0.6);
		} else {
			glColor4f(0.0, 1.0, 0.0, 0.6);
		}
		GLOOPushedMatrix pm;
		glRotatef(_uprightness*90, 1, 0, 0);
		const dReal* p = dGeomGetOffsetPosition(get_entity().get_geom("run_detect"));
		glTranslatef(p[0], p[1], p[2]);
		gluSphere(q, RUNNING_SPHERE_RAD, 20, 10);
		
		gluDeleteQuadric(q);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
	}
	
	if (_run_coll_occurred) {
		_run_coll_occurred = false;
	}
}

AvatarGameObj::AvatarGameObj(const ORE1::ObjType& obj) :
	GameObj(obj, Sim::gen_sphere_body(80, 0.5)), // TODO Load mass information from the ORE mission description
	_anim_fly_to_prerun(MeshAnimation::load("action-LIBAvatar-Run")),
	_run_coll_occurred(false),
	_attached(false)
{
	// Set up a geom for detecting regular collisions
	// TODO Load volume information from the ORE mission description
	get_entity().set_geom(
		"physical",
		dCreateCCylinder(Sim::get_dyn_space(), 0.25, 2.0),
		std::auto_ptr<CollisionHandler>(new SimpleContactHandler)
	);
	
	// Set up a geom at our feet to detect when we can run on a surface
	get_entity().set_geom(
		"run_detect",
		dCreateSphere(Sim::get_dyn_space(), RUNNING_SPHERE_RAD),
		std::auto_ptr<CollisionHandler>(new RunCollisionTracker)
	);
	
	// TODO Maybe some missions start off in upright mode?
	_uprightness = 0.0;
}