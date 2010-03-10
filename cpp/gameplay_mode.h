/*
gameplay_mode.h: Header for the GameplayMode class.
The GameplayMode class is the active Mode during normal gameplay.

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

#ifndef ORBIT_RIBBON_GAMEPLAY_MODE_H
#define ORBIT_RIBBON_GAMEPLAY_MODE_H

#include "mode.h"

class AvatarGameObj;

class GameplayMode : public Mode {
	private:
		std::string _avatar_key;
		
		AvatarGameObj* find_avatar();
		
	public:
		GameplayMode();
		void pre_3d();
		void draw_3d_far();
		void draw_3d_near();
		void draw_2d();
};


#endif