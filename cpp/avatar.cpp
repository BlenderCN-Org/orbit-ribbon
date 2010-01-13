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

#include "autoxsd/orepkgdesc.h"
#include "avatar.h"
#include "geometry.h"

// Camera positioning relative to avatar's reference frame
const Vector CAMERA_POS_OFFSET(0.0, 1.1, -6.0);
const Vector CAMERA_TGT_OFFSET(0.0, 1.1, 0.0);
const Vector CAMERA_UP_VECTOR(0.0, 1.0, 0.0);

GOAutoRegistration<AvatarGameObj> avatar_gameobj_reg("Avatar");

void AvatarGameObj::step_impl() {
}

void AvatarGameObj::near_draw_impl() {
}

AvatarGameObj::AvatarGameObj(const ORE1::ObjType& obj) : GameObj(obj) {
}