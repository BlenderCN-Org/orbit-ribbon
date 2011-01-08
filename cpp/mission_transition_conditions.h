/*
mission_transition_conditions.h: Header for conditions determining mission-state transitions.
These determine what needs to happen to move from one state to another in a mission's FSM.

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

#ifndef ORBIT_RIBBON_MISSION_TRANSITION_CONDITIONS_H
#define ORBIT_RIBBON_MISSION_TRANSITION_CONDITIONS_H

#include "geometry.h"
#include "mission_fsm.h"

namespace ORE1 { class RingsPassedConditionType; }
class RingsPassedCondition : public MissionStateTransitionCondition {
  private:
    unsigned int _rings;

    unsigned int passed_rings() const;

  public:
    RingsPassedCondition(const ORE1::RingsPassedConditionType& condition);
    void draw_impl(const GameplayMode& gameplay_mode);
    bool is_true(const GameplayMode& gameplay_mode);
};

namespace ORE1 { class TimerCountdownConditionType; }
class TimerCountdownCondition : public MissionStateTransitionCondition {
  private:
    unsigned int _nanvi;
    unsigned int _steps_at_start;
    bool _started;

    float elapsed_nanvi() const;

  public:
    TimerCountdownCondition(const ORE1::TimerCountdownConditionType& condition);
    void draw_impl(const GameplayMode& gameplay_mode);
    bool is_true(const GameplayMode& gameplay_mode);
};

namespace ORE1 { class AvatarMovesConditionType; }
class AvatarMovesCondition : public MissionStateTransitionCondition {
  private:
    Point _starting_pos;
    bool _started;

  public:
    AvatarMovesCondition(const ORE1::AvatarMovesConditionType& condition);
    bool is_true(const GameplayMode& gameplay_mode);
};

#endif
