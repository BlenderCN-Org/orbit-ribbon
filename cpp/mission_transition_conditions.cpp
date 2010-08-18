/*
mission_transition_conditions.cpp: Implementation for conditions determining mission-state transitions.
These determine what needs to happen to move from one state to another in a mission's FSM.

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

#include "mission_transition_conditions.h"
#include "autoxsd/orepkgdesc.h"

AutoRegistrationBySourceTypename<
  MissionStateTransitionConditionFactorySpec,
  RingsPassedCondition,
  ORE1::RingsPassedConditionType
> rings_passed_condition_reg;

RingsPassedCondition::RingsPassedCondition(const ORE1::RingsPassedConditionType& condition) :
  MissionStateTransitionCondition(condition)
{
}

bool RingsPassedCondition::is_true() {
  return false;
}

AutoRegistrationBySourceTypename<
  MissionStateTransitionConditionFactorySpec,
  TimerCountdownCondition,
  ORE1::TimerCountdownConditionType
> timer_countdown_condition_reg;

TimerCountdownCondition::TimerCountdownCondition(const ORE1::TimerCountdownConditionType& condition) :
  MissionStateTransitionCondition(condition)
{
}

bool TimerCountdownCondition::is_true() {
  return false;
}

AutoRegistrationBySourceTypename<
  MissionStateTransitionConditionFactorySpec,
  AvatarMovesCondition,
  ORE1::AvatarMovesConditionType
> avatar_moves_condition_reg;

AvatarMovesCondition::AvatarMovesCondition(const ORE1::AvatarMovesConditionType& condition) :
  MissionStateTransitionCondition(condition)
{
}

bool AvatarMovesCondition::is_true() {
  return false;
}