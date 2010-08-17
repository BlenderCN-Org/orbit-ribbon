/*
mission_fsm.h: Header for mission-related finite state machines.
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

#ifndef ORBIT_RIBBON_MISSION_FSM_H
#define ORBIT_RIBBON_MISSION_FSM_H

#include <boost/shared_ptr.hpp>
#include <list>
#include <map>
#include <string>

#include "factory.h"

namespace ORE1 {
  class MissionStateType;
  class MissionStateTransitionType;
  class MissionType;
  class MissionConditionType;
  class MissionEffectType;
}

class MissionEffect {
  public:
    virtual void entering_state() {}
    virtual void process_frame() {}
    virtual void exiting_state() {}
    virtual void draw() {}
};

class MissionEffectFactorySpec :
  public FactorySpecBase<MissionEffect, ORE1::MissionEffectType> {};

class MissionStateTransitionCondition {
  private:
    bool _display;
  
  protected:
    virtual void draw_impl() {}
  
  public:
    MissionStateTransitionCondition(bool display) : _display(display) {}
    virtual bool is_true() =0;
    void draw() { if (_display) draw_impl(); }
};

class MissionStateTransitionConditionFactorySpec :
  public FactorySpecBase<MissionStateTransitionCondition, ORE1::MissionConditionType> {};

class MissionStateTransition {
  private:
    typedef std::list<boost::shared_ptr<MissionStateTransitionCondition> > ConditionList;
    ConditionList _conditions;
    std::string _target_name;
  
  public:
    MissionStateTransition(const ORE1::MissionStateTransitionType& transition);
    
    std::string get_target_name() const { return _target_name; }
    bool conditions_true() const;
    void draw();
};

class MissionState {
  private:
    typedef std::list<boost::shared_ptr<MissionEffect> > EffectList;
    EffectList _effects;
    std::list<MissionStateTransition> _transitions;
  
  public:
    MissionState(const ORE1::MissionStateType& state);
    
    std::string get_transition();
    
    virtual void entering_state();
    virtual void process_frame();
    virtual void exiting_state();
    virtual void draw();
};

class MissionFSM {
  private:
    typedef std::map<std::string, MissionState> StateMap;
    StateMap _states;
    MissionState* _cur_state;
    
    void transition_to_state(const std::string& name);
  
  public:
    MissionFSM(const ORE1::MissionType& mission);
    void process_frame();
    void draw();
};

#endif