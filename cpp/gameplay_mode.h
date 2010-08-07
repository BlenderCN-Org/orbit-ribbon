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
    
    bool simulation_enabled() { return true; }
    
    // There is no input handling phase in this mode because that's dealt with in avatar step instead
    // If it weren't handled in avatar step, control would be negatively affected by framerate drops
    
    void pre_clear(bool top);
    void pre_3d(bool top);
    void draw_3d_far(bool top);
    void draw_3d_near(bool top);
    void draw_2d(bool top);
};


#endif
