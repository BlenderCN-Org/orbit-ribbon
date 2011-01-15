/*
mission_fsm.h: Implementation for mission-related finite state machines.
The mission FSM handles progression through a mission's objectives.

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

#include <boost/foreach.hpp>

#include "autoxsd/orepkgdesc.h"
#include "debug.h"
#include "except.h"
#include "gameplay_mode.h"
#include "globals.h"
#include "simple_menu_modes.h"

#include "mission_fsm.h"

MissionEffect::MissionEffect(const ORE1::MissionEffectType& effect __attribute__ ((unused))) {
}

MissionStateTransitionCondition::MissionStateTransitionCondition(const ORE1::MissionConditionType& condition) :
  _display(condition.display())
{  
}

MissionStateTransition::MissionStateTransition(const ORE1::MissionStateTransitionType& transition) :
  _target_name(transition.target())
{
  ORE1::MissionStateTransitionType::cond_const_iterator i;
  for (i = transition.cond().begin(); i != transition.cond().end(); ++i) {
    _conditions.push_back(get_factory<MissionStateTransitionConditionFactorySpec>().create(*i));
  }
}

bool MissionStateTransition::conditions_true(const GameplayMode& gameplay_mode) const {
 BOOST_FOREACH(const boost::shared_ptr<MissionStateTransitionCondition>& _cond, _conditions) {
   if (!_cond->is_true(gameplay_mode)) {
     return false;
   }
 }
 return true;
}

void MissionStateTransition::draw(const GameplayMode& gameplay_mode) {
  BOOST_FOREACH(const boost::shared_ptr<MissionStateTransitionCondition>& _cond, _conditions) {
    _cond->draw(gameplay_mode);
  }
}

MissionState::MissionState(const ORE1::MissionStateType& state) {
  ORE1::MissionStateType::effect_const_iterator i;
  for (i = state.effect().begin(); i != state.effect().end(); ++i) {
    _effects.push_back(get_factory<MissionEffectFactorySpec>().create(*i));
  }
  
  ORE1::MissionStateType::transition_const_iterator j;
  for (j = state.transition().begin(); j != state.transition().end(); ++j) {
    _transitions.push_back(MissionStateTransition(*j));
  }
}

std::string MissionState::get_transition(const GameplayMode& gameplay_mode) {
  BOOST_FOREACH(const MissionStateTransition& transition, _transitions) {
    if (transition.conditions_true(gameplay_mode)) {
      return transition.get_target_name();
    }
  }
  return "";
}

void MissionState::entering_state(const GameplayMode& gameplay_mode) {
  BOOST_FOREACH(const boost::shared_ptr<MissionEffect>& effect, _effects) {
    effect->entering_state(gameplay_mode);
  }
}

void MissionState::step(const GameplayMode& gameplay_mode) {
  BOOST_FOREACH(const boost::shared_ptr<MissionEffect>& effect, _effects) {
    effect->step(gameplay_mode);
  }
}

void MissionState::exiting_state(const GameplayMode& gameplay_mode) {
  BOOST_FOREACH(const boost::shared_ptr<MissionEffect>& effect, _effects) {
    effect->exiting_state(gameplay_mode);
  }
}

void MissionState::draw(const GameplayMode& gameplay_mode) {
  BOOST_FOREACH(const boost::shared_ptr<MissionEffect>& effect, _effects) {
    effect->draw(gameplay_mode);
  }
  BOOST_FOREACH(MissionStateTransition& transition, _transitions) {
    transition.draw(gameplay_mode);
  }
}

void MissionFSM::transition_to_state(const std::string& name) { 
  Debug::status_msg("Entering mission state \"" + name + "\"");

  if (_cur_state.get() != NULL) {
    _cur_state->exiting_state(_gameplay_mode);
  }

  if (name == "win" or name == "fail") {
    _finished = true;
    Globals::mode_stack->next_frame_push_mode(boost::shared_ptr<Mode>(new PostMissionMenuMode(name == "win")));
    return;
  }

  ORE1::MissionType::state_const_iterator i;
  for (i = _mission.state().begin(); i != _mission.state().end(); ++i) {
    if (i->name() == name) { break; }
  }
  if (i == _mission.state().end()) {
    throw GameException("No such mission state \"" + name + "\"");
  }
  _cur_state.reset(new MissionState(*i));
  _cur_state->entering_state(_gameplay_mode); 
}

MissionFSM::MissionFSM(const ORE1::MissionType& mission, const GameplayMode& gameplay_mode) :
  _mission(mission), _gameplay_mode(gameplay_mode), _cur_state(NULL), _finished(false)
{
}

void MissionFSM::step() {
  if (_finished) {
    return;
  }

  if (_cur_state.get() == NULL) {
    transition_to_state("start");
  }
  
  _cur_state->step(_gameplay_mode);
  
  std::string transition_target = _cur_state->get_transition(_gameplay_mode);
  if (transition_target.size() > 0) {
    transition_to_state(transition_target);
  }
}

void MissionFSM::draw() {
  if (_cur_state.get() == NULL) {
    transition_to_state("start");
  }
  
  _cur_state->draw(_gameplay_mode);
}
