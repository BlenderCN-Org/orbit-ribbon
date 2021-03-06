/*
mission_effects.h: Header for mission-state effects.
These determine what happens at any particular state in a mission's FSM.

Copyright 2011 David Simon <david.mike.simon@gmail.com>

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

#ifndef ORBIT_RIBBON_MISSION_EFFECTS_H
#define ORBIT_RIBBON_MISSION_EFFECTS_H

#include "mission_fsm.h"

namespace ORE1 { class DisplayMessageEffectType; }
class DisplayMessageEffect : public MissionEffect {
  public:
    DisplayMessageEffect(const ORE1::DisplayMessageEffectType& effect);
};

namespace ORE1 { class StartTimerEffectType; }
class StartTimerEffect : public MissionEffect {
  public:
    StartTimerEffect(const ORE1::StartTimerEffectType& effect);
};

#endif