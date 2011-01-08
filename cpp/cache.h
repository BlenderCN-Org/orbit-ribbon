/*
cache.h: Header and implementation for cache base class.
Provides a mechanism to allow re-using objects which have already been loaded.

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

#ifndef ORBIT_RIBBON_CACHE_H
#define ORBIT_RIBBON_CACHE_H

#include <string>
#include <boost/unordered_map.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>

template<typename T> class CacheBase {
  private:
    typedef boost::unordered_map<std::string, boost::weak_ptr<T> > CacheMap;
    CacheMap _cache;
    
  public:
    virtual boost::shared_ptr<T> generate(const std::string& id) =0;
    
    boost::shared_ptr<T> get(const std::string& id) {
      boost::weak_ptr<T>& p = _cache[id];
      if (p.expired()) {
        boost::shared_ptr<T> gen = generate(id);
        p = gen;
        return gen;
      } else {
        return p.lock();
      }
    }
};

#endif
