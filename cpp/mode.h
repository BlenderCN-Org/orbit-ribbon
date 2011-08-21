/*
mode.h: Header for the Mode class and ModeStack class.
Mode classes are responsible for handling overall control of gameplay and menu behaviour

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

#ifndef ORBIT_RIBBON_MODE_H
#define ORBIT_RIBBON_MODE_H

#include <string>
#include <stack>
#include <queue>
#include <boost/shared_ptr.hpp>

class ModeStack;

class Mode {
  public:
    // Default is to be inconclusive about wanting the simulation to run.
    // This causes the choice to trickle down to the next mode.
    virtual bool simulation_disabled() { return false; }
    virtual bool simulation_enabled() { return false; }
    
    virtual bool execute_after_lower_mode() { return false; }
    virtual bool mouse_cursor_enabled() { return false; }
    
    // ModeStack will descend down the modes calling handle_input until it finds one that returns true
    virtual bool handle_input() { return false; }
    
    virtual void set_camera(bool top __attribute__ ((unused))) {}
    virtual void draw_3d_far(bool top __attribute__ ((unused))) {}
    virtual void draw_3d_near(bool top __attribute__ ((unused))) {}
    virtual void draw_2d(bool top __attribute__ ((unused))) {}
    
    virtual void step() {}
    
    virtual void pushed_below_top() {}
    virtual void now_at_top() {}

    virtual void prior_top(const boost::shared_ptr<Mode>& m __attribute__ ((unused))) {}
};

class ModeStack {
  private:
    class PoppedModeStackItem {
      private:
        ModeStack* _mode_stack;
      public:
        boost::shared_ptr<Mode> mode;
        
        PoppedModeStackItem(ModeStack& mode_stack) : _mode_stack(&mode_stack), mode(mode_stack._stack.top()) {
          mode_stack._stack.pop();
        }
        ~PoppedModeStackItem() { _mode_stack->_stack.push(mode); }
    };
    
    class Operation {
      public:
        virtual void apply(ModeStack& mode_stack) =0;
    };
    
    class PushOperation : public Operation {
      private:
        boost::shared_ptr<Mode> _mode_to_push;
      
      public:
        PushOperation(const boost::shared_ptr<Mode>& mode) : _mode_to_push(mode) {}
        void apply(ModeStack& mode_stack);
    };
    
    class PopOperation : public Operation {
      public:
        void apply(ModeStack& mode_stack);
    };
    
    friend class Operation;
    
    std::stack<boost::shared_ptr<Mode> > _stack;
    std::queue<boost::shared_ptr<Operation> > _op_queue;
    bool _mouse_inactive;
    
    void execute_input_handling_phase();
    void execute_simulation_phase(unsigned int steps_elapsed);
    void execute_camera_phase(bool top);
    void execute_draw_phase(bool top);
  
  public:
    ModeStack() : _mouse_inactive(true) {}
    
    void next_frame_push_mode(const boost::shared_ptr<Mode>& new_mode);
    void next_frame_pop_mode();
    
    void execute_frame(unsigned int steps_elapsed);
};

#endif
