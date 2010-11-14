/*
main_modes.cpp: Header for interpolation mode classes.
These handle smooth transitions between two different modes.

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

#ifndef ORBIT_RIBBON_INTERP_MODE_H
#define ORBIT_RIBBON_INTERP_MODE_H

#include <boost/shared_ptr.hpp>
#include <SDL/SDL.h>

#include "globals.h"
#include "gloo.h"
#include "mode.h"

template <template <typename> class InterpFunctor> class InterpolationMode : public Mode {
  private:
    GLOOCamera _camera;
    InterpFunctor<GLOOCamera> _camera_interpolator;
    boost::shared_ptr<Mode> _src, _tgt;
    unsigned int _ms, _start;
    bool _started, _reverse;

  public:
    InterpolationMode(unsigned int ms, const boost::shared_ptr<Mode>& tgt) : _tgt(tgt), _ms(ms), _started(false), _reverse(false) {}

    const GLOOCamera* get_camera(bool top __attribute__ ((unused))) {
      if (!_started) {
        _started = true;
        _start = SDL_GetTicks();
      }

      float mu = float(SDL_GetTicks()-_start)/_ms;
      if (mu >= 1.0) {
        if (_reverse) {
          Globals::mode_stack.next_frame_pop_mode();
          _camera = *(_src->get_camera(true));
        } else {
          Globals::mode_stack.next_frame_push_mode(_tgt);
          _camera = *(_tgt->get_camera(false));
        }
      } else {
        _camera = _camera_interpolator(*(_src->get_camera(true)), *(_tgt->get_camera(false)), _reverse ? 1.0 - mu : mu);
      }
      return &_camera;
    }

    void draw_3d_far(bool top __attribute__ ((unused))) {
      _tgt->draw_3d_far(false);
      _tgt->draw_3d_near(false);
    }

    void draw_3d_near(bool top __attribute__ ((unused))) {
      _tgt->draw_3d_near(false);
    }

    void pushed_below_top() {
      _reverse = true;
      _started = false;
    }

    void prior_top(const boost::shared_ptr<Mode>& m) {
      _src = m;
    }
};

#endif
