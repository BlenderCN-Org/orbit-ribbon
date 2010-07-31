/*
saving.cpp: Implementation for classes managing saving/loading the config.

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
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

#include "debug.h"
#include "except.h"
#include "saving.h"

boost::scoped_ptr<boost::filesystem::path> Saving::_save_path;
boost::scoped_ptr<ORSave::SaveType> Saving::_save;

ORSave::SaveType& Saving::get() {
  if (_save) { return *_save; }
  throw GameException("Attempted to get save data before it had been loaded.");
}

ORSave::InputDeviceType& Saving::get_input_device(ORSave::InputDeviceNameType::Value dev_type) {
  BOOST_FOREACH(ORSave::InputDeviceType& idev, get().config().inputDevice()) {
    if (idev.device() == dev_type) {
      return idev;
    }
  }
  throw GameException("Unable to locate input device config in save file matching requested type");
}

template<typename AT, typename VT> void conf_dflt(AT& optional_attr, const VT& default_value) {
  if (!optional_attr.present()) {
    optional_attr.set(default_value);
  }
}

void Saving::load() {
  if (!_save_path) {
    // TODO Choose different locations on other OSes
    const char* home_loc = std::getenv("HOME");
    if (!home_loc) {
      throw GameException("Unable to find HOME environment to locate save file");
    }
    _save_path.reset(new boost::filesystem::path(std::string(home_loc) + "/.orbit-ribbon"));
  }
  
  std::ifstream ifs(_save_path->string().c_str());
  if (ifs) {
    // Load the save data from this file
    Debug::status_msg("Loading save data from '" + _save_path->string() + "'");
    try {
      // TODO Turn on validation (need to compile in the XSD...)
      _save.reset(ORSave::save(ifs, xsd::cxx::tree::flags::dont_validate).release());
    } catch (const xml_schema::Exception& e) {
      std::stringstream ss;
      ss << e;
      throw GameException(std::string("XML error while loading save data : ") + ss.str());
    } catch (const std::exception& e) {
      throw GameException(std::string("Error while loading save data : ") + e.what());
    }  
  } else {
    // Create a new, empty save
    Debug::status_msg("Creating new save data");
    _save.reset(new ORSave::SaveType(ORSave::ConfigType()));
  }
  
  // Load default config values for anything unspecified
  ORSave::ConfigType* conf = &(_save->config());
  conf_dflt(conf->lastOre(), "no-lack-of-sky.ore");
  conf_dflt(conf->showFps(), false);
  conf_dflt(conf->fullScreen(), true);
  conf_dflt(conf->vSync(), true);
  conf_dflt(conf->debugPhysics(), false);
  conf_dflt(conf->soundEffectVolume(), 0.8);
  conf_dflt(conf->musicVolume(), 0.5);
  conf_dflt(conf->mouseSensitivity(), 0.5);
  conf_dflt(conf->invertTranslateY(), false);
  conf_dflt(conf->invertRotateY(), false);
}

void Saving::save() {
  if (!_save) {
    throw GameException("Attempted to save when no save data was available");
  }
  
  Debug::status_msg("Writing save data to '" + _save_path->string() + "'");
  xml_schema::NamespaceInfomap map;
  map["orsave"].name = "http://www.orbit-ribbon.org/ORSave";
  std::ofstream ofs(_save_path->string().c_str());
  ORSave::save(ofs, *_save, map);
}
