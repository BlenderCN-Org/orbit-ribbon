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

// How many seconds it takes to move between 0.0 and 1.0 uprightness, and therefore how much _uprightness change each step
const float UPRIGHTNESS_ENTRY_TIME = 0.5;
const float UPRIGHTNESS_STEP_DIFF = 1.0f/(UPRIGHTNESS_ENTRY_TIME*MAX_FPS);

// Maximum amount per second that each of these avatar-relative values can be changed for aligning with a running surface
const float RUNNING_ADJ_RATE_X_ROT = 0.3; // Radians
const float RUNNING_ADJ_RATE_Z_ROT = 0.3; // Radians
const float RUNNING_ADJ_RATE_Y_POS = 0.2; // Meters
const float RUNNING_ADJ_RATE_Y_LVEL = 5.0; // Meters per second
const float RUNNING_ADJ_RATE_X_AVEL = 5.0; // Radians per second
const float RUNNING_ADJ_RATE_Z_AVEL = 5.0; // Radians per second

// If it would take longer than this number of seconds to fully adjust one of the above values, don't attach to the running surface
const float RUNNING_MAX_ADJUST_TIME = 1.3;

// If our uprightness is not at least at this value, do not run
const float RUNNING_MIN_UPRIGHTNESS = 0.9;

// Radius of the sphere used to ignore contacts around our feet when we run
const float RUNNING_NOCOLL_SPHERE_RAD = 0.35;

GOAutoRegistration<AvatarGameObj> avatar_gameobj_reg("Avatar");

bool AvatarGameObj::AvatarContactHandler::handle_collision(float t __attribute__ ((unused)), dGeomID o __attribute__ ((unused)), const dContactGeom* c, unsigned int c_len)  {
	Point p = _avatar->get_rel_point_pos(Point(dGeomGetOffsetPosition(_avatar->get_entity().get_geom("run_detect"))));
	
	if (_avatar->_attached) {
		for (unsigned int i = 0; i < c_len; ++i) {
			if (p.dist_to(Point(c[i].pos)) > RUNNING_NOCOLL_SPHERE_RAD) {
				Debug::debug_msg("C ACCEPT");
				return true;
			}
		}
		Debug::debug_msg("C REJECT");
		return false;
	} else {
		Debug::debug_msg("C STD");
		return true;
	}
}

bool AvatarGameObj::RunCollisionTracker::should_contact(float t __attribute__ ((unused)), dGeomID o __attribute__ ((unused)), const dContactGeom* c __attribute__ ((unused)), unsigned int c_len __attribute__ ((unused))) const {
	return false;
}

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
	
	// Update the orientation of our physical geom to match _uprightness
	dMatrix3 grot;
	dRFromAxisAndAngle(grot, 1, 0, 0, _uprightness*M_PI_2);
	dGeomSetOffsetRotation(get_entity().get_geom("physical"), grot);
	
	// Check if we are contacting a surface in a way that allows attaching
	_attached = false;
	RunCollisionTracker* rct = static_cast<RunCollisionTracker*>(get_entity().get_geom_ch("run_detect"));
	if (rct->has_collisions()) {
		std::auto_ptr<std::vector<CollisionTracker::Collision> > colls = rct->get_collisions();
		
		// TODO Consider using two contact points, one for each foot, then averaging out the plane normal
		// TODO Check if the other object is runnable/attachable
		// TODO Shouldn't just assume that first contact of first collision is the best one to use
		Vector sn = Vector(colls->back().contacts[0].normal); // Surface normal
		Vector sn_rel = vector_from_world(sn);
		float depth = colls->back().contacts[0].depth;
		
		// TODO Maybe allow a special "walking" or "crawling" attachment when relative velocity to surface is very low
		// TODO Detach (or fail to attach) when relative velocity is non-trivial and too different from facing direction
		// TODO Treat linear velocity projected onto XZ as another one of these limited variables, to simulate friction and limit max run speed
		if (_uprightness >= RUNNING_MIN_UPRIGHTNESS) {
			Vector lvel = Vector(dBodyGetLinearVel(body));
			Vector lvel_rel = vector_from_world(lvel);
			Vector avel = Vector(dBodyGetAngularVel(body));
			Vector avel_rel = vector_from_world(avel);
			
			// Offsets to various values that we'd like to apply for optimal running state
			float xrot_delta = -std::asin(sn_rel.z); // Rotate about x axis to reduce z to 0
			float zrot_delta = -std::asin(sn_rel.x); // Rotate about z axis to reduce x to 0
			float ypos_delta = -depth + RUNNING_ADJ_RATE_Y_POS * RUNNING_MAX_ADJUST_TIME;
			float ylvel_delta = -lvel_rel.y;
			float xavel_delta = -avel_rel.x;
			float zavel_delta = -avel_rel.z;
			
			/*
			Debug::debug_msg(std::string("D:") +
				" XROT:" + boost::lexical_cast<std::string>(xrot_delta) +
				" ZROT:" + boost::lexical_cast<std::string>(zrot_delta) +
				" YPOS:" + boost::lexical_cast<std::string>(ypos_delta) +
				" YLVL:" + boost::lexical_cast<std::string>(ylvel_delta) +
				" XAVL:" + boost::lexical_cast<std::string>(xavel_delta) +
				" ZAVL:" + boost::lexical_cast<std::string>(zavel_delta)
			);
			*/
			
			// If the difference between current and ideal state is not too severe, attach to surface
			if (
				std::fabs(xrot_delta) <= RUNNING_ADJ_RATE_X_ROT*RUNNING_MAX_ADJUST_TIME &&
				std::fabs(zrot_delta) <= RUNNING_ADJ_RATE_Z_ROT*RUNNING_MAX_ADJUST_TIME &&
				std::fabs(ypos_delta) <= RUNNING_ADJ_RATE_Y_POS*RUNNING_MAX_ADJUST_TIME &&
				std::fabs(ylvel_delta) <= RUNNING_ADJ_RATE_Y_LVEL*RUNNING_MAX_ADJUST_TIME &&
				std::fabs(xavel_delta) <= RUNNING_ADJ_RATE_X_AVEL*RUNNING_MAX_ADJUST_TIME &&
				std::fabs(zavel_delta) <= RUNNING_ADJ_RATE_Z_AVEL*RUNNING_MAX_ADJUST_TIME
			) {
				_attached = true;
				
				// Okay, apply as much of each delta as we can
				
				// X and Z orientation delta
				// TODO Maybe should translate body so that the contact point stays in the same spot through rotation
				float a = limit_abs(zrot_delta, RUNNING_ADJ_RATE_Z_ROT/MAX_FPS);
				Vector body_x(vector_to_world(Vector(cos(a), sin(a), 0)));
				a = limit_abs(-xrot_delta, RUNNING_ADJ_RATE_X_ROT/MAX_FPS);
				Vector body_y(vector_to_world(Vector(0, cos(a), sin(a))));
				dMatrix3 matr;
				dRFrom2Axes(matr, body_x.x, body_x.y, body_x.z, body_y.x, body_y.y, body_y.z);
				dBodySetRotation(body, matr);
				
				// Y position delta
				set_pos(get_pos() + sn*limit_abs(ypos_delta, RUNNING_ADJ_RATE_Y_POS/MAX_FPS));
				
				// Y linear velocity delta
				lvel_rel.y += limit_abs(ylvel_delta, RUNNING_ADJ_RATE_Y_LVEL/MAX_FPS);
				lvel = vector_to_world(lvel_rel);
				dBodySetLinearVel(body, lvel.x, lvel.y, lvel.z);
				
				// X and Z angular velocity delta
				avel_rel.x += limit_abs(xavel_delta, RUNNING_ADJ_RATE_X_AVEL/MAX_FPS);
				avel_rel.z += limit_abs(zavel_delta, RUNNING_ADJ_RATE_Z_AVEL/MAX_FPS);
				avel = vector_to_world(avel_rel);
				dBodySetAngularVel(body, avel.x, avel.y, avel.z);
			}
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
		
		if (_uprightness >= RUNNING_MIN_UPRIGHTNESS) { 
			if (_attached) {
				glColor4f(1.0, 0.0, 0.0, 0.6);
			} else {
				glColor4f(0.0, 1.0, 0.0, 0.6);
			}
			GLOOPushedMatrix pm;
			glTranslatef(0, 0, -_height/2 - (RUNNING_ADJ_RATE_Y_POS*RUNNING_MAX_ADJUST_TIME));
			gluCylinder(q, 0.05, 0.05, (RUNNING_ADJ_RATE_Y_POS * RUNNING_MAX_ADJUST_TIME * 2), 10, 10);
		}
		
		gluDeleteQuadric(q);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
	}
}

AvatarGameObj::AvatarGameObj(const ORE1::ObjType& obj) :
	GameObj(obj, Sim::gen_sphere_body(80, 0.5)), // TODO Load mass information from the ORE mission description
	_anim_fly_to_prerun(MeshAnimation::load("action-LIBAvatar-Run")),
	_attached(false)
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
	
	// Set up a geom at our feet to detect when we can run on a surface
	get_entity().set_geom(
		"run_detect",
		dCreateRay(Sim::get_dyn_space(), RUNNING_ADJ_RATE_Y_POS * RUNNING_MAX_ADJUST_TIME * 2),
		std::auto_ptr<CollisionHandler>(new RunCollisionTracker)
	);
	dQuaternion rdq;
	dQFromAxisAndAngle(rdq, 1, 0, 0, M_PI_2);
	dGeomSetOffsetQuaternion(get_entity().get_geom("run_detect"), rdq);
	dGeomSetOffsetPosition(get_entity().get_geom("run_detect"), 0, -_height/2 + (RUNNING_ADJ_RATE_Y_POS*RUNNING_MAX_ADJUST_TIME), 0);
	
	// TODO Maybe some missions start off in upright mode?
	_uprightness = 0.0;
}