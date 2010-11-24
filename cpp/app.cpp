/*
app.cpp: Implementation of the App class
App is responsible for the main frame loop and calling all the other parts of the program.

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

#include <GL/glew.h>
#include <GL/gl.h>
#include <SDL/SDL.h>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

#include "autofont/latinmodern.h"
#include "autoinfo/version.h"
#include "app.h"
#include "background.h"
#include "constants.h"
#include "debug.h"
#include "display.h"
#include "except.h"
#include "font.h"
#include "gameplay_mode.h"
#include "geometry.h"
#include "globals.h"
#include "gameobj.h"
#include "input.h"
#include "menu_modes.h"
#include "mesh.h"
#include "mode.h"
#include "mouse_cursor.h"
#include "performance.h"
#include "saving.h"
#include "sim.h"
#include "ore.h"

// How often in ticks to update the performance info string
const unsigned int MAX_PERF_INFO_AGE = 200;

void App::frame_loop() {
  std::string perf_info;
  unsigned int last_perf_info = 0; // Tick time at which we last updated perf_info
  
  unsigned int unsimulated_ticks = 0;
  
  while (1) {
    GLint frame_start = SDL_GetTicks();
    
    // Fetch the latest state of the input devices
    Input::update();
    
    // Add all new events to the events list for this frame, move mouse cursor, and quit on QUIT events
    Globals::frame_events.clear();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      Globals::frame_events.push_back(event);
      if (event.type == SDL_QUIT) {
        throw GameQuitException("SDL quit event");
      }
    }
    
    // Handle mouse cursor movement, and showing/hiding the mouse cursor as necessary
    Globals::mouse_cursor->process_events();
    
    // Check for a ForceQuit binding activation
    const Channel& force_quit_chn = Input::get_button_ch(ORSave::ButtonBoundAction::ForceQuit);
    if (force_quit_chn.is_on()) {
      throw GameQuitException("ForceQuit binding " + force_quit_chn.desc() + " activated");
    }
    
    // Check for a ResetNeutral binding activation
    const Channel& reset_neutral_chn = Input::get_button_ch(ORSave::ButtonBoundAction::ResetNeutral);
    if (reset_neutral_chn.is_on()) {
      Debug::status_msg("ResetNeutral binding " + reset_neutral_chn.desc() + " activated");
      Input::set_neutral();
    }
    
    // This is where the important stuff happens, depending on what mode the game currently is
    unsigned int steps_to_simulate = unsimulated_ticks / MIN_TICKS_PER_FRAME;
    Globals::mode_stack.execute_frame(steps_to_simulate);
    unsimulated_ticks -= steps_to_simulate * MIN_TICKS_PER_FRAME;
    
    // If showFps config flag is enabled, calculate and display recent FPS
    if (Saving::get().config().showFps().get()) {
      if (perf_info == "" or SDL_GetTicks() - last_perf_info >= MAX_PERF_INFO_AGE) {
        last_perf_info = SDL_GetTicks();
        perf_info = Performance::get_perf_info() + " " + GLOOBufferedMesh::get_usage_info();
      }
      const static Point pos(15, 15);
      const static float height = 15;
      GUI::draw_diamond_box(Box(pos, Size(Globals::sys_font->get_width(height, perf_info), height) + GUI::DIAMOND_BOX_BORDER*2));
      glColor3f(1.0, 1.0, 1.0);
      Globals::sys_font->draw(pos + GUI::DIAMOND_BOX_BORDER, height, perf_info);
    }
    
    // Output frame and flip buffers
    SDL_GL_SwapBuffers();
    
    // Sleep if we're running faster than our maximum fps
    unsigned int frame_ticks = SDL_GetTicks() - frame_start;
    if (frame_ticks > 0 && frame_ticks < MIN_TICKS_PER_FRAME) {
      SDL_Delay(MIN_TICKS_PER_FRAME - frame_ticks); // Slow down, buster!
    }
    
    // The time that this frame took to run needs to pass in the simulator next frame
    // TODO: Put a maximum cap on how much can be added to this each frame prevent runaway simulation
    unsimulated_ticks += SDL_GetTicks() - frame_start;
    
    // Put this frame's timing data into the FPS calculations
    unsigned int total_ticks = SDL_GetTicks() - frame_start;
    Performance::record_frame(total_ticks, total_ticks - frame_ticks);
  }
}

void App::run(const std::vector<std::string>& args) {
  try {
    // Parse command-line arguments
    boost::program_options::options_description visible_opt_desc("Command-line options");
    visible_opt_desc.add_options()
      ("help,h", "display help message, then exit")
      ("version,V", "display version and author information, then exit")
      ("fullscreen,f", "run game in fullscreen mode (defaults to native resolution)")
      ("windowed,w", "run game in windowed mode (defaults to 800x600)")
      ("area,a", boost::program_options::value<unsigned int>(), "preselect numbered area to play")
      ("mission,m", boost::program_options::value<unsigned int>(), "preselect numbered mission to play, you must also specify --area")
    ;
    boost::program_options::options_description hidden_opt_desc;
    hidden_opt_desc.add_options()
      ("ore", boost::program_options::value<std::string>(), "path to an ore file containing the game data and scenario to use")
    ;
    boost::program_options::options_description opt_desc;
    opt_desc.add(visible_opt_desc).add(hidden_opt_desc);
    boost::program_options::positional_options_description pos_desc;
    pos_desc.add("ore", 1);
    
    boost::program_options::variables_map vm;
    try {
      boost::program_options::store(boost::program_options::command_line_parser(args).options(opt_desc).positional(pos_desc).run(), vm);
      boost::program_options::notify(vm);
      if (vm.count("mission") and not vm.count("area")) {
        throw GameException("mission specified but no area specified");
      }
    } catch (const std::exception& e) {
      Debug::error_msg(std::string("Invalid arguments: ") + e.what());
      return;
    }
    
    // Deal with command-line arguments that cause the program to end right away
    if (vm.count("help") or vm.count("version")) {
      Debug::status_msg("");
      Debug::status_msg(std::string("Orbit Ribbon version ") + APP_VERSION);
      Debug::status_msg("");
      if (vm.count("help")) {
        std::stringstream ss;
        visible_opt_desc.print(ss);
        Debug::status_msg(ss.str());
      } else {
        Debug::status_msg(std::string("Compiled: ") + BUILD_DATE);
        if (std::strlen(COMMIT_HASH) > 0) {
          Debug::status_msg(std::string("Commit Hash: ") + COMMIT_HASH);
          Debug::status_msg(std::string("Commit Date: ") + COMMIT_DATE);
        }
        Debug::status_msg("");
        Debug::status_msg("Copyright 2009 David Simon, who can be reached at <david.mike.simon@gmail.com>.");
        Debug::status_msg("License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.");
        Debug::status_msg("This is free software: you are free to change and redistribute it.");
        Debug::status_msg("");
        Debug::status_msg("Orbit Ribbon is distributed in the hope that it will be awesome,");
        Debug::status_msg("but WITHOUT ANY WARRANTY; without even the implied warranty of");
        Debug::status_msg("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the");
        Debug::status_msg("GNU General Public License for more details.");
        Debug::status_msg("");
      }
      return;
    }
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
      throw GameException(std::string("SDL initialization failed: ") + std::string(SDL_GetError()));
    }
 
    Saving::load();

    // Set to windowed or fullscreen if the appropriate command line arguments were given
    if (vm.count("fullscreen") or vm.count("windowed")) {
      Saving::get().config().fullScreen().set((bool)vm.count("fullscreen"));
      
      // This forces Display::init to pick a new, sane resolution
      Saving::get().config().screenWidth().set(0);
      Saving::get().config().screenHeight().set(0);
    }
    
    Display::init();
    Background::init();
    
    boost::filesystem::path orePath;
    bool orePathSave = false;
    if (vm.count("ore")) {
      orePathSave = true;
      orePath = boost::filesystem::system_complete(vm["ore"].as<std::string>());
    } else {
      orePath = boost::filesystem::path(Saving::get().config().lastOre().get());
    }
    try {
      Debug::status_msg("Loading ORE package '" + orePath.file_string() + "'");
      Globals::ore.reset(new OrePackage(orePath));
    } catch (const OreException& e) {
      // TODO: Display a dialog to the user that lets them pick a different ORE file
      throw;
    }
    if (orePathSave) {
      //If a different ORE file was successfully loaded, save that path to the config
      Saving::get().config().lastOre().set(orePath.file_string());
      Saving::save();
    }

    // Find all the libscenes in this ore and let them be easily accessible by name
    const ORE1::PkgDescType* desc = &Globals::ore->get_pkg_desc();
    for (ORE1::PkgDescType::LibsceneConstIterator i = desc->libscene().begin(); i != desc->libscene().end(); ++i) {
      Globals::libscenes.insert(LSMap::value_type(i->name(), *i));
    }
    
    Sim::init();
    Input::init();
    
    Globals::sys_font.reset(new Font(FONTDATA_LATINMODERN, FONTDATA_LATINMODERN_LEN, FONTDATA_LATINMODERN_DESC));
    Globals::bg.reset(new Background);
    Globals::mouse_cursor.reset(new MouseCursor());
    
    if (vm.count("area") and vm.count("mission")) {
      unsigned int area = vm["area"].as<unsigned int>();
      unsigned int mission = vm["mission"].as<unsigned int>();
      if (area < 1 || mission < 1) {
        throw GameException("Area and mission numbers must be positive integers");
      }
      load_mission(area, mission);
      Globals::mode_stack.next_frame_push_mode(boost::shared_ptr<Mode>(new GameplayMode()));
    } else if (vm.count("area")) {
      unsigned int area = vm["area"].as<unsigned int>();
      if (area < 1) {
        throw GameException("Area and mission numbers must be positive integers");
      }
      load_mission(area, 0);
      Globals::mode_stack.next_frame_push_mode(boost::shared_ptr<Mode>(new MainMenuMode()));
      Globals::mode_stack.next_frame_push_mode(boost::shared_ptr<Mode>(new AreaSelectMenuMode()));
      Globals::mode_stack.next_frame_push_mode(boost::shared_ptr<Mode>(new MissionSelectMenuMode(area)));
    } else {
      Globals::mode_stack.next_frame_push_mode(boost::shared_ptr<Mode>(new MainMenuMode()));
    }
  } catch (const std::exception& e) {
    Debug::error_msg(std::string("Uncaught exception during init: ") + e.what());
    return;
  }
  
  try {
    frame_loop();
  } catch (const GameQuitException& e) {
    Debug::status_msg(std::string("Program quitting: ") + e.what());
  } catch (const std::exception& e) {
    Debug::error_msg(std::string("Uncaught exception during run: ") + e.what());
  }
  
  Saving::save();
  
  Debug::status_msg("Thanks for playing!");
  
  // TODO Deinitialize as well as possible here, to reduce possibility of weird error messages on close
}

void App::load_mission(unsigned int area_num, unsigned int mission_num) {
  if (mission_num > 0) {
    Debug::status_msg(
      "Loading area " + boost::lexical_cast<std::string>(area_num) +
      ", mission " + boost::lexical_cast<std::string>(mission_num)
    );
  } else {
    Debug::status_msg("Loading area " + boost::lexical_cast<std::string>(area_num));
  }
  
  const ORE1::PkgDescType* desc = &Globals::ore->get_pkg_desc();
  
  const ORE1::AreaType* area = NULL;
  for (ORE1::PkgDescType::AreaConstIterator i = desc->area().begin(); i != desc->area().end(); ++i) {
    if (i->n() == area_num) {
      area = &(*i);
      break;
    }
  }
  if (!area) {
    throw GameException(
      "Unable to load area " + boost::lexical_cast<std::string>(area_num)
    );
  }
  
  const ORE1::MissionType* mission = NULL;
  if (mission_num > 0) {
    for (ORE1::AreaType::MissionConstIterator i = area->mission().begin(); i != area->mission().end(); ++i) {
      if (i->n() == mission_num) {
        mission = &(*i);
        break;
      }
    }
    if (!mission) {
      throw GameException(
        "Unable to load mission " + boost::lexical_cast<std::string>(mission_num) + " from area " + boost::lexical_cast<std::string>(area_num)
      );
    }
  }
  
  // TODO: Instead of clearing gameobjs, maybe keep the old one around a while so that meshes and textures stay loaded.
  // Though, watch out for running out of video memory at point where both lists fully loaded...
  // Also watch out for fragmentation of VBO space; at the moment GLOO doesn't have any way of dealing with that.
  // Easiest solution: Notice if we are changing missions within an area, then keep all objs temp loaded, and...
  // If we are reloading the same area and mission, keep everything temp loaded.
  // NOTE: Cannot just leave objects in gameobjs, they may have changed state during gameplay.
  // Another possible idea: Have orbit-edit.py notice common objects/textures among all or nearly all missions, then we
  // can load them on ORE load.
  Globals::gameobjs.clear();
  for (ORE1::AreaType::ObjConstIterator i = area->obj().begin(); i != area->obj().end(); ++i) {
    Globals::gameobjs.insert(GOMap::value_type(i->objName(), get_factory<GameObjFactorySpec>().create(*i)));
  }
  if (mission) {
    for (ORE1::MissionType::ObjConstIterator i = mission->obj().begin(); i != mission->obj().end(); ++i) {
      Globals::gameobjs.insert(GOMap::value_type(i->objName(), get_factory<GameObjFactorySpec>().create(*i)));
    }
  }
  
  Globals::bg->set_sky(area->sky());
  
  Globals::current_area = area;
  Globals::current_mission = mission;
}
