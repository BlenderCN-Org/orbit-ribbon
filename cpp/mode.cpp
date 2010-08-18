/*
mode.cpp: Implementation for the Mode class and ModeStack class.
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

#include "mode.h"

#include "constants.h"
#include "display.h"
#include "except.h"
#include "globals.h"
#include "mouse_cursor.h"
#include "sim.h"

void ModeStack::PushOperation::apply(ModeStack& mode_stack) {
  if (!mode_stack._stack.empty()) {
    mode_stack._stack.top()->pushed_below_top();
  }
  _mode_to_push->now_at_top();
  mode_stack._stack.push(_mode_to_push);
}

void ModeStack::PopOperation::apply(ModeStack& mode_stack) {
  if (!mode_stack._stack.empty()) {
    mode_stack._stack.pop();
  }
  if (!mode_stack._stack.empty()) {
    mode_stack._stack.top()->now_at_top();
  }
}

void ModeStack::next_frame_push_mode(const boost::shared_ptr<Mode>& new_mode) {
  _op_queue.push(boost::shared_ptr<PushOperation>(new PushOperation(new_mode)));
}

void ModeStack::next_frame_pop_mode() {
  _op_queue.push(boost::shared_ptr<PopOperation>(new PopOperation()));
}

void ModeStack::execute_input_handling_phase() {
  PoppedModeStackItem cur_mode(*this);
  
  // If this mode handles input, then stop descending
  if (cur_mode.mode->handle_input()) {
    return;
  }
  
  // If this mode doesn't handle input, descend down to the next one
  if (!_stack.empty()) {
    execute_input_handling_phase();
  }
}

void ModeStack::execute_simulation_phase(unsigned int steps_elapsed) {
  PoppedModeStackItem cur_mode(*this);
  
  // If this mode blocks simulation, then just stop here, don't simulate or descend any further
  if (cur_mode.mode->simulation_disabled()) {
    return;
  }
  
  // If this mode wants simulation, then run the simulation and stop descending here
  if (cur_mode.mode->simulation_enabled()) {
    // Do a simulation step for each realtime tick elapsed
    for (; steps_elapsed > 0; --steps_elapsed) {
      cur_mode.mode->step();
      Sim::sim_step();
    }
    return;
  }
  
  // If it's inconclusive, see if the next mode down wants to block or run simulation
  if (!_stack.empty()) {
    execute_simulation_phase(steps_elapsed);
  }
}

void ModeStack::execute_pre_clear_phase(bool top) {
  PoppedModeStackItem cur_mode(*this);

  if (cur_mode.mode->execute_after_lower_mode() && !_stack.empty()) {
    // Descend recursively if this mode wants the prior mode ran first
    execute_pre_clear_phase(false);
  }
  
  cur_mode.mode->pre_clear(top);
}

void ModeStack::execute_draw_phase(bool top) {
  PoppedModeStackItem cur_mode(*this);
    
  if (cur_mode.mode->execute_after_lower_mode() && !_stack.empty()) {
    // Descend recursively if this mode wants the prior mode ran first
    execute_draw_phase(false);
  }
  
  // Set up 3D drawing mode (projection matrix will be set below)
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  
  cur_mode.mode->pre_3d(top);
  
  // Projection mode for distant objects
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV, Display::get_screen_ratio(), 0.1, SKY_CLIP_DIST);
  glMatrixMode(GL_MODELVIEW);
  
  cur_mode.mode->draw_3d_far(top);
  
  // Projection mode for nearby objects
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV, Display::get_screen_ratio(), 0.1, GAMEPLAY_CLIP_DIST);
  glMatrixMode(GL_MODELVIEW);
  
  cur_mode.mode->draw_3d_near(top);
  
  // 2D drawing mode
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, Display::get_screen_width(), Display::get_screen_height(), 0.0); // Set origin at top-left of screen
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  cur_mode.mode->draw_2d(top);
  
  // The top mode gets to decide if the mouse cursor is drawn
  if (top) {
    if (cur_mode.mode->mouse_cursor_enabled()) {
      if (_mouse_inactive) {
        _mouse_inactive = false;
        Globals::mouse_cursor->reset_pos();
      }
      Globals::mouse_cursor->draw();
    } else if (!_mouse_inactive) {
      Globals::mouse_cursor->set_visibility(false);
      _mouse_inactive = true;
    }
  }
}

void ModeStack::execute_frame(unsigned int steps_elapsed) {
  while (!_op_queue.empty()) {
    _op_queue.front()->apply(*this);
    _op_queue.pop();
  }
  
  if (_stack.empty()) {
    throw GameQuitException("Mode stack depleted");
  } else {
    execute_input_handling_phase();
    execute_simulation_phase(steps_elapsed);
    execute_pre_clear_phase(true);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    execute_draw_phase(true);
  }
}