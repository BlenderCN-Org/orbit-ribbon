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
#include <boost/filesystem/fstream.hpp>
#include <sstream>

#include "autoxsd/save-pimpl.h"
#include "autoxsd/save-simpl.h"
#include "constants.h"
#include "debug.h"
#include "except.h"
#include "globals.h"
#include "saving.h"

boost::scoped_ptr<ORSave::SaveType> Saving::_save;

boost::filesystem::path Saving::save_path() {
  return Globals::save_dir / SAVE_FILENAME;
}

ORSave::SaveType& Saving::get() {
  if (_save) { return *_save; }
  throw GameException("Attempted to get save data before it had been loaded.");
}

ORSave::InputDeviceType& Saving::get_input_device(ORSave::InputDeviceNameType::value_type dev_type) {
  BOOST_FOREACH(ORSave::InputDeviceType& idev, get().config().inputDevice()) {
    if (idev.device() == dev_type) {
      return idev;
    }
  }
  throw GameException("Unable to locate input device config in save file matching requested type");
}

#define CONF_DFLT(C, P, A, V) if (!C.P()) { C.A(V); }
void Saving::load() {
  boost::filesystem::ifstream ifs(save_path());
  if (ifs) {
    // Load the save data from this file
    Debug::status_msg("Loading save data from '" + save_path().string() + "'");
    try {
      ORSave::save_paggr save_p;
      xml_schema::document_pimpl doc_p(save_p.root_parser(), save_p.root_namespace(), save_p.root_name(), true);
      save_p.pre();
      doc_p.parse(ifs);
      _save.reset(save_p.post());
    } catch (const xml_schema::parser_exception& e) {
      throw GameException(std::string("Parsing problem while loading save data : ") + e.text());
    } catch (const std::exception& e) {
      throw GameException(std::string("Error while loading save data : ") + e.what());
    }  
  } else {
    // Create a new, empty save
    Debug::status_msg("Creating new save data");
    _save.reset(new ORSave::SaveType());
    _save->config(new ORSave::ConfigType());
  }
  
  // Load default config values for anything unspecified
  // TODO: See if it is possible to do this in the xsd instead
  ORSave::ConfigType& conf = _save->config();
  CONF_DFLT(conf, lastOre_present, lastOre, "main.ore");
  CONF_DFLT(conf, showFps_present, showFps, true);
  CONF_DFLT(conf, fullScreen_present, fullScreen, false);
  CONF_DFLT(conf, vSync_present, vSync, true);
  CONF_DFLT(conf, debugPhysics_present, debugPhysics, false);
  CONF_DFLT(conf, soundEffectVolume_present, soundEffectVolume, 0.8);
  CONF_DFLT(conf, musicVolume_present, musicVolume, 0.5);
  CONF_DFLT(conf, mouseSensitivity_present, mouseSensitivity, 0.5);
  CONF_DFLT(conf, invertTranslateY_present, invertTranslateY, false);
  CONF_DFLT(conf, invertRotateY_present, invertRotateY, false);
}

void Saving::save() {
  if (!_save) {
    throw GameException("Attempted to save when no save data was available");
  }
  
  Debug::status_msg("Writing save data to '" + save_path().string() + "'");
  boost::filesystem::ofstream ofs(save_path());
  ORSave::save_saggr save_s;
  xml_schema::document_simpl doc_s(save_s.root_serializer(), save_s.root_namespace(), save_s.root_name(), true);
  doc_s.add_prefix("orsave", save_s.root_namespace());
  save_s.pre(*_save);
  doc_s.serialize(ofs, xml_schema::document_simpl::pretty_print);
  save_s.post();
}
