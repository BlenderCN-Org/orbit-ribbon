/*
mission_fsm.h: Implementation for mission-related finite state machines.
The mission FSM handles progression through a mission's objectives.

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

#include <boost/foreach.hpp>

#include "autoxsd/orepkgdesc.h"
#include "except.h"

#include "mission_fsm.h"

MissionStateTransition::MissionStateTransition(const ORE1::MissionStateTransitionType& transition) :
  _target_name(transition.target())
{
  ORE1::MissionStateTransitionType::CondConstIterator i;
  for (i = transition.cond().begin(); i != transition.cond().end(); ++i) {
    _conditions.push_back(get_factory<MissionStateTransitionConditionFactorySpec>().create(*i));
  }
}

bool MissionStateTransition::conditions_true() const {
 BOOST_FOREACH(const boost::shared_ptr<MissionStateTransitionCondition>& _cond, _conditions) {
   if (!_cond->is_true()) {
     return false;
   }
 }
 return true;
}

void MissionStateTransition::draw() {
  BOOST_FOREACH(const boost::shared_ptr<MissionStateTransitionCondition>& _cond, _conditions) {
    _cond->draw();
  }
}

MissionState::MissionState(const ORE1::MissionStateType& state) {
  ORE1::MissionStateType::EffectConstIterator i;
  for (i = state.effect().begin(); i != state.effect().end(); ++i) {
    _effects.push_back(get_factory<MissionEffectFactorySpec>().create(*i));
  }
  
  ORE1::MissionStateType::TransitionConstIterator j;
  for (j = state.transition().begin(); j != state.transition().end(); ++j) {
    _transitions.push_back(MissionStateTransition(*j));
  }
}

std::string MissionState::get_transition() {
  BOOST_FOREACH(const MissionStateTransition& transition, _transitions) {
    if (transition.conditions_true()) {
      return transition.get_target_name();
    }
  }
  return "";
}

void MissionState::entering_state() {
  BOOST_FOREACH(const boost::shared_ptr<MissionEffect>& effect, _effects) {
    effect->entering_state();
  }
}

void MissionState::process_frame() {
  BOOST_FOREACH(const boost::shared_ptr<MissionEffect>& effect, _effects) {
    effect->process_frame();
  }
}

void MissionState::exiting_state() {
  BOOST_FOREACH(const boost::shared_ptr<MissionEffect>& effect, _effects) {
    effect->exiting_state();
  }
}

void MissionState::draw() {
  BOOST_FOREACH(const boost::shared_ptr<MissionEffect>& effect, _effects) {
    effect->draw();
  }
  BOOST_FOREACH(MissionStateTransition& transition, _transitions) {
    transition.draw();
  }
}

void MissionFSM::transition_to_state(const std::string& name) {
  StateMap::iterator i = _states.find(name);
  if (i == _states.end()) {
    throw GameException("Unable to transition to state \"" + name + "\"");
  }
  if (_cur_state != NULL) {
    _cur_state->exiting_state();
  }
  _cur_state = &(i->second);
  _cur_state->entering_state(); 
}

MissionFSM::MissionFSM(const ORE1::MissionType& mission) : _cur_state(NULL) {
  ORE1::MissionType::StateConstIterator i;
  for (i = mission.state().begin(); i != mission.state().end(); ++i) {
    _states.insert(StateMap::value_type(i->name(), MissionState(*i)));
  }
}

void MissionFSM::process_frame() {
  if (_cur_state == NULL) {
    transition_to_state("start");
  }
  
  _cur_state->process_frame();
  
  std::string transition_target = _cur_state->get_transition();
  if (transition_target.size() > 0) {
    transition_to_state(transition_target);
  }
}

void MissionFSM::draw() {
  if (_cur_state == NULL) {
    transition_to_state("start");
  }
  
  _cur_state->draw();
}