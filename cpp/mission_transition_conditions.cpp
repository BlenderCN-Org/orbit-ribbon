/*
mission_transition_conditions.cpp: Implementation for conditions determining mission-state transitions.
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

#include "mission_transition_conditions.h"

#include "autoxsd/orepkgdesc.h"
#include "avatar.h"
#include "constants.h"
#include "font.h"
#include "gameplay_mode.h"
#include "globals.h"
#include "gloo.h"
#include "target_ring.h"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

const float AVATAR_MOVES_CONDITION_DISTANCE = 1.5;

AutoRegistrationBySourceTypename<
  MissionStateTransitionConditionFactorySpec,
  RingsPassedCondition,
  ORE1::RingsPassedConditionType
> rings_passed_condition_reg;

unsigned int RingsPassedCondition::passed_rings() const {
  unsigned int ret = 0;
  for (GOMap::iterator i = Globals::gameobjs.begin(); i != Globals::gameobjs.end(); ++i) {
    GOMap::size_type idx = i->first.find("LIBTargetRing");
    if (idx == 0) {
      if (static_cast<const TargetRingGameObj*>(&*(i->second))->passed()) {
        ++ret;
      }
    }
  }
  return ret;
}

RingsPassedCondition::RingsPassedCondition(const ORE1::RingsPassedConditionType& condition) :
  MissionStateTransitionCondition(condition),
  _rings(condition.rings())
{
}

void RingsPassedCondition::draw_impl(const GameplayMode& gameplay_mode) {
  std::string s = boost::str(boost::format("%u djine velvi'u") % (_rings - passed_rings()));
  Point pos = gameplay_mode.get_condition_widget_pos(Size(Globals::sys_font->get_width(20, s), 20));
  Globals::sys_font->draw(pos, 20, s);
}

bool RingsPassedCondition::is_true(const GameplayMode& gameplay_mode __attribute__ ((unused))) {
  return passed_rings() >= _rings;
}

AutoRegistrationBySourceTypename<
  MissionStateTransitionConditionFactorySpec,
  TimerCountdownCondition,
  ORE1::TimerCountdownConditionType
> timer_countdown_condition_reg;

TimerCountdownCondition::TimerCountdownCondition(const ORE1::TimerCountdownConditionType& condition) :
  MissionStateTransitionCondition(condition),
  _nanvi(condition.nanvi()),
  _steps_at_start(0),
  _started(false)
{
}

float TimerCountdownCondition::elapsed_nanvi() const {
  return ((Globals::total_steps - _steps_at_start)/float(MAX_FPS))*NANVI_PER_SECOND;
}

void TimerCountdownCondition::draw_impl(const GameplayMode& gameplay_mode) {
  std::string s = boost::str(boost::format("%.2f nanvi velvi'u") % (_nanvi - elapsed_nanvi()));
  Point pos = gameplay_mode.get_condition_widget_pos(Size(Globals::sys_font->get_width(20, s), 20));
  Globals::sys_font->draw(pos, 20, s);
}

bool TimerCountdownCondition::is_true(const GameplayMode& gameplay_mode __attribute__ ((unused))) {
  if (!_started) {
    _steps_at_start = Globals::total_steps;
    _started = true;
  }
  
  return elapsed_nanvi() > _nanvi;
}

AutoRegistrationBySourceTypename<
  MissionStateTransitionConditionFactorySpec,
  AvatarMovesCondition,
  ORE1::AvatarMovesConditionType
> avatar_moves_condition_reg;

AvatarMovesCondition::AvatarMovesCondition(const ORE1::AvatarMovesConditionType& condition) :
  MissionStateTransitionCondition(condition),
  _started(false)
{
}

bool AvatarMovesCondition::is_true(const GameplayMode& gameplay_mode) {
  if (!_started) {
    _starting_pos = gameplay_mode.find_avatar()->get_pos();
    _started = true;
  }
  return gameplay_mode.find_avatar()->get_pos().dist_to(_starting_pos) > AVATAR_MOVES_CONDITION_DISTANCE;
}
