/*
mode.h: Implementation of the Mode class and its subclasses.
GameMode classes are responsible for handling overall control of gameplay and menu behaviour

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
#include <GL/glu.h>
#include <boost/format.hpp>
#include <cmath>

#include "avatar.h"
#include "background.h"
#include "constants.h"
#include "display.h"
#include "except.h"
#include "gameplay_mode.h"
#include "globals.h"
#include "gameobj.h"
#include "geometry.h"
#include "gui.h"
#include "saving.h"

// Camera positioning relative to avatar's reference frame
const Vector CAMERA_POS_OFFSET(0.0, 1.1, -7.0);
const Vector CAMERA_TGT_OFFSET(0.0, 1.1, 0.0);
const Vector CAMERA_UP_VECTOR(0.0, 1.0, 0.0);

// Where and how to draw the physics debugging info box
const float PHYS_DEBUG_BOX_Y = -100;
const Size PHYS_DEBUG_BOX_SIZE(400, 80);
const std::string PHYS_DEBUG_BOX_NUMFMT("%+5.3f");
const float PHYS_DEBUG_BOX_FONTSIZE = 15;

GameplayMode::GameplayMode() {
	// Locate the avatar object
	for (GOMap::iterator i = Globals::gameobjs.begin(); i != Globals::gameobjs.end(); ++i) {
		GOMap::size_type idx = i->first.find("LIBAvatar");
		if (idx == 0) {
			_avatar_key = i->first;
			break;
		}
	}
	
	if (_avatar_key.size() == 0) {
		throw GameException(std::string("Unable to locate LIBAvatar GameObj in GameplayMode init!"));
	}
}

AvatarGameObj* GameplayMode::find_avatar() {
	GOMap::iterator i = Globals::gameobjs.find(_avatar_key);
	if (i == Globals::gameobjs.end()) {
		throw GameException(std::string("GameplayMode: LIBAvatar GameObj named ") + _avatar_key + " has disappeared unexpectedly");
	}
	return static_cast<AvatarGameObj*>(&(*(i->second)));
}

void GameplayMode::pre_3d() {
	AvatarGameObj* avatar = find_avatar();
	Point cam_pos = avatar->get_rel_point_pos(CAMERA_POS_OFFSET);
	Point cam_tgt = avatar->get_rel_point_pos(CAMERA_TGT_OFFSET);
	Vector up_vec = avatar->vector_to_world(CAMERA_UP_VECTOR);
	gluLookAt(
		cam_pos.x, cam_pos.y, cam_pos.z,
		cam_tgt.x, cam_tgt.y, cam_tgt.z,
		 up_vec.x,  up_vec.y,  up_vec.z
	);
}

void GameplayMode::draw_3d_far() {
	// Draw all the background objects
	Globals::bg->draw();
}

void GameplayMode::draw_3d_near() {
	// Draw every game object (FIXME Do near/far sorting)
	for (GOMap::iterator i = Globals::gameobjs.begin(); i != Globals::gameobjs.end(); ++i) {
		i->second->draw(true);
	}
}

void GameplayMode::draw_2d() {
	AvatarGameObj* av = find_avatar();
	
	if (Saving::get().config().debugPhysics().get()) {
		Point p(
			Display::get_screen_width()/2 - PHYS_DEBUG_BOX_SIZE.x/2,
			Display::get_screen_height()/2 - PHYS_DEBUG_BOX_SIZE.y/2 + PHYS_DEBUG_BOX_Y
		);
		Gui::draw_box(p, PHYS_DEBUG_BOX_SIZE + GUI_BOX_BORDER*2);
		p += GUI_BOX_BORDER;
		
		static const char* labels[6] = { "XROT", "ZROT", "YPOS", "XAVEL", "ZAVEL", "YLVEL"};
		float cur_vals[6] = {
			av->get_last_coll_xrot(),
			av->get_last_coll_zrot(),
			av->get_last_coll_ypos(),
			av->get_last_coll_xavel(),
			av->get_last_coll_zavel(),
			av->get_last_coll_ylvel()
		};
		float max_vals[6] = {
			RUNNING_MAX_DELTA_X_ROT,
			RUNNING_MAX_DELTA_Z_ROT,
			RUNNING_MAX_DELTA_Y_POS,
			RUNNING_MAX_DELTA_Y_LVEL,
			RUNNING_MAX_DELTA_X_AVEL,
			RUNNING_MAX_DELTA_Z_AVEL
		};
		
		
		boost::format f(PHYS_DEBUG_BOX_NUMFMT);
		for (int i = 0; i < 6; ++i) {
			float x = (PHYS_DEBUG_BOX_SIZE.x/6) * i;
			glColor3f(1.0, 1.0, 1.0);
			Globals::sys_font->draw(p + Vector(x, 0), PHYS_DEBUG_BOX_FONTSIZE, labels[i]);
			Globals::sys_font->draw(p + Vector(x, PHYS_DEBUG_BOX_FONTSIZE*2), PHYS_DEBUG_BOX_FONTSIZE, (f % max_vals[i]).str());
			
			float r = std::fabs(cur_vals[i])/(max_vals[i]*1.2);
			glColor3f(1.0, 1.0 - r, 1.0 - r);
			Globals::sys_font->draw(p + Vector(x, PHYS_DEBUG_BOX_FONTSIZE), PHYS_DEBUG_BOX_FONTSIZE, (f % cur_vals[i]).str());
		}
	}
}