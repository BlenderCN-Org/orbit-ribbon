/*
interpolation.h: Header for template interpolation functions 
Interpolation is used for smoothly animating characters, cameras, etc. between keyframes

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

#ifndef ORBIT_RIBBON_INTERPOLATION_H
#define ORBIT_RIBBON_INTERPOLATION_H

#include <cmath>

// This takes advantage of how annoying it would be to make this general to accept references.
// The in-place linear interpolation makes internal copies, but this version takes advantage of the copies being made anyways.
template <typename T> struct Interpolator {
  T operator()(T y0, T y1, float mu) {
    // Default implementation is linear interpolation
    y0 *= (1-mu);
    y1 *= mu;
    y0 += y1;
    return y0;
  }
};

template <typename T> struct CosineInterpolator : public Interpolator<T> {
  T operator()(T y0, T y1, float mu) {
    return Interpolator<T>::operator()(y0, y1, (1-std::cos(mu*M_PI))/2);
  }
};

template <typename T> struct SquareInterpolator : public Interpolator<T> {
  T operator()(T y0, T y1, float mu) {
    return Interpolator<T>::operator()(y0, y1, 1 - mu*mu);
  }
};

template <typename T> struct QuadInterpolator : public Interpolator<T> {
  T operator()(T y0, T y1, float mu) {
    return Interpolator<T>::operator()(y0, y1, 1 - mu*mu*mu*mu);
  }
};

template <template <typename> class I> struct Reverser {
  template <typename T> struct Reversed {
    I<T> orig;
    T operator()(T y0, T y1, float mu) {
      return orig(y0, y1, 1 - mu);
    }
  };
};

#endif
