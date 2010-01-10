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

#include "constants.h"
#include "except.h"
#include "mode.h"
#include "globals.h"
#include "gameobj.h"

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

void GameplayMode::set_camera() {
	GameObj* avatar;
	try {
		avatar = &Globals::gameobjs.at(_avatar_key);
	} catch (const boost::bad_ptr_container_operation& e) {
		throw GameException(std::string("GameplayMode: LIBAvatar GameObj named ") + _avatar_key + " has disappeared unexpectedly");
	}
	
	// FIXME Just testing here, really need to do this within avatar's reference frame
	gluLookAt(
		avatar->get_pos().x, avatar->get_pos().y, avatar->get_pos().z,
		0.0, 0.0, 0.0,
		GAMEPLAY_CAMERA_UP_VECTOR.x, GAMEPLAY_CAMERA_UP_VECTOR.y, GAMEPLAY_CAMERA_UP_VECTOR.z
	);
}