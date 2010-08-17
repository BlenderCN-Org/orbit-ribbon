/*
factory.h: Template class for generating an object based on string-specified class.

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

#ifndef ORBIT_RIBBON_FACTORY_H
#define ORBIT_RIBBON_FACTORY_H

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include <memory>
#include <utility>
#include <typeinfo>

#include "except.h"

template<class BaseType, class SourceType> class FactorySpecBase {
  public:
    typedef BaseType base_type;
    typedef SourceType source_type;
    
    virtual std::string extract_name(const SourceType& source) { return typeid(source).name(); }
};

template<class FactorySpec> class Generator {
  public:
    virtual boost::shared_ptr<typename FactorySpec::base_type> create(const typename FactorySpec::source_type& source) =0;
};

template<class FactorySpec, class C> class AutoGenerator : public Generator<FactorySpec> {
  public:
    virtual boost::shared_ptr<typename FactorySpec::base_type> create(const typename FactorySpec::source_type& source) {
      return boost::shared_ptr<typename FactorySpec::base_type>(new C(source));
    }
};

template<class FactorySpec> class Factory {
  public:  
    // The entry with the key "" is special: it's the default, used if no more specific generator could be found
    typedef boost::shared_ptr<Generator<FactorySpec> > GeneratorPtr;
    typedef std::map<std::string, GeneratorPtr> GeneratorMap;
    GeneratorMap _generator_map;
    
    void register_class(const GeneratorPtr& ptr, const std::string& name = "") {
      typename GeneratorMap::iterator i = _generator_map.find(name);
      if (i != _generator_map.end()) {
        throw GameException("Attempted to register over existing factory for " + name);
      }
      _generator_map.insert(typename GeneratorMap::value_type(name, ptr));
    }
    
    boost::shared_ptr<typename FactorySpec::base_type> create(const typename FactorySpec::source_type& source) const {
      FactorySpec s;
      std::string name = s.extract_name(source);
      typename GeneratorMap::const_iterator i = _generator_map.find(name);
      if (i == _generator_map.end()) {
        i = _generator_map.find("");
      }
      
      if (i != _generator_map.end()) {
        return i->second->create(source);
      } else {
        throw GameException("No implementation for " + name + " under FactorySpec " + typeid(FactorySpec).name());
      }
    }
};

// Hack to avoid initialization order problems.
// We cannot create static instances of Factory and then use static instances of AutoRegistration to fill them out.
// We couldn't be sure the Factory would be initialized first if that happened.
// This function takes care of it; the factory is created as soon as it is needed, no sooner and no later.
template<class FactorySpec> Factory<FactorySpec>& get_factory() {
  static Factory<FactorySpec> factory;
  return factory;
}

template<class FactorySpec, class C> class AutoRegistration {
  public:
    AutoRegistration(const std::string& name = "") {
      boost::shared_ptr<Generator<FactorySpec> > generator;
      generator.reset(new AutoGenerator<FactorySpec, C>);
      get_factory<FactorySpec>().register_class(generator , name);
    }
};

template<class FactorySpec, class C> class AutoDefaultRegistration : public AutoRegistration<FactorySpec, C> {};

#endif