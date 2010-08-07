/*
mode.h: Header for the Mode class and ModeStack class.
Mode classes are responsible for handling overall control of gameplay and menu behaviour

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
    
    virtual void pre_clear(bool top __attribute__ ((unused))) {}
    virtual void pre_3d(bool top __attribute__ ((unused))) {}
    virtual void draw_3d_far(bool top __attribute__ ((unused))) {}
    virtual void draw_3d_near(bool top __attribute__ ((unused))) {}
    virtual void draw_2d(bool top __attribute__ ((unused))) {}
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
    
    void execute_simulation_phase(unsigned int ticks_elapsed);
    void execute_pre_clear_phase(bool top);
    void execute_draw_phase(bool top);
  
  public:
    void next_frame_push_mode(const boost::shared_ptr<Mode>& new_mode);
    void next_frame_pop_current_mode();
    
    void execute_frame(unsigned int ticks_elapsed);
};

#endif