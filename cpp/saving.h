/*
saving.h: Header for classes managing saving/loading the config.

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

#ifndef ORBIT_RIBBON_SAVING_H
#define ORBIT_RIBBON_SAVING_H

#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>

#include "autoxsd/save.h"

class Saving {
  private:
    static boost::scoped_ptr<ORSave::SaveType> _save;

    static boost::filesystem::path save_path();

  public:
    static ORSave::SaveType& get();
    static ORSave::ConfigType::inputDevice_iterator get_input_device(ORSave::InputDeviceNameType::value_type dev_type);

    static void load();
    static void save();
};

#endif
