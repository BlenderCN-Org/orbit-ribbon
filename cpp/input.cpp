/*
input.cpp: Implementation of input-management classes.
These classes are responsible for taking input from the keyboard and gamepad, and binding input to game actions.

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

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <cctype>
#include <cmath>
#include <sstream>
#include <typeinfo>

#include "autoxsd/presets.xml.h"
#include "autoxsd/save.h"
#include "autoxsd/save-pimpl.h"
#include "constants.h"
#include "debug.h"
#include "except.h"
#include "globals.h"
#include "input.h"
#include "saving.h"

// How far from -1, 0, or 1 where we consider an input axis to just be at those exact values
const float DEAD_ZONE = 0.001;

// At maximum and minimum sensitivity, how far the mouse has to move in a single step to be equivalent to full axis tilt
const float MAX_MOUSE_SENSITIVITY_FULL = 5;
const float MIN_MOUSE_SENSITIVITY_FULL = 50;

bool Channel::is_partially_on() const {
  return is_on();
}

void Channel::set_neutral() {
  // Do nothing by default.
}

bool Channel::is_null() const {
  return false;
}

void ChannelSource::set_neutral() {
  BOOST_FOREACH(boost::shared_ptr<Channel> ch, _channels) {
    ch->set_neutral();
  }
}

Keyboard::Keyboard() {
  update();
  for (unsigned int i = (const unsigned int)SDLK_FIRST; i < (const unsigned int)SDLK_LAST; ++i) {
    _channels.push_back(boost::shared_ptr<Channel>(new KeyChannel(this, SDLKey(i))));
  }
}

void Keyboard::update() {
  _sdl_key_state = SDL_GetKeyState(NULL);
}

const boost::shared_ptr<Channel> Keyboard::key_channel(SDLKey key) const {
  return _channels[(unsigned int)key];
}

Mouse::Mouse() {
  update();
  for (Uint8 a = 0; a < 2; ++a) {
    _axis_map[a] = _channels.size();
    _channels.push_back(boost::shared_ptr<Channel>(new MouseMovementChannel(this, a)));
  }
  for (Uint8 b = 1; b <= 7; ++b) { // Since SDL_GetRelativeMouseState returns a Uint8 mask, and 1st button is 1, max of 7 buttons suppoted
    _button_map[b] = _channels.size();
    _channels.push_back(boost::shared_ptr<Channel>(new MouseButtonChannel(this, b)));
  }
}

void Mouse::update() {
  _btn_mask = SDL_GetRelativeMouseState(&_rel_x, &_rel_y);
}

GamepadManager::GamepadManager() {
  for (Uint8 i = 0; i < SDL_NumJoysticks(); ++i) {
    SDL_Joystick* gp = SDL_JoystickOpen(i);
    _gamepads.push_back(gp);
    for (Uint8 a = 0; a < SDL_JoystickNumAxes(gp); ++a) {
      _gamepad_axis_map[i][a] = _channels.size();
      _channels.push_back(boost::shared_ptr<Channel>(new GamepadAxisChannel(this, i, a)));
    }
    for (Uint8 b = 0; b < SDL_JoystickNumButtons(gp); ++b) {
      _gamepad_button_map[i][b] = _channels.size();
      _channels.push_back(boost::shared_ptr<Channel>(new GamepadButtonChannel(this, i, b)));
    }
  }
  update();
}

GamepadManager::~GamepadManager() {
  for (unsigned int i = 0; i < _gamepads.size(); ++i) {
    SDL_JoystickClose(_gamepads[i]);
  }
}

void GamepadManager::update() {
  SDL_JoystickUpdate();
}

const boost::shared_ptr<Channel> Mouse::button_channel(Uint8 btn) const {
  std::map<Uint8, unsigned int>::const_iterator i = _button_map.find(btn);
  if (i == _button_map.end()) {
    return Input::get_null_channel();
  }
  return _channels[i->second];
}

const boost::shared_ptr<Channel> Mouse::movement_channel(Uint8 btn) const {
  std::map<Uint8, unsigned int>::const_iterator i = _axis_map.find(btn);
  if (i == _axis_map.end()) {
    return Input::get_null_channel();
  }
  return _channels[i->second];
}

const boost::shared_ptr<Channel> GamepadManager::axis_channel(Uint8 gamepad_num, Uint8 axis_num) const {
  std::map<Uint8, std::map<Uint8, unsigned int> >::const_iterator i = _gamepad_axis_map.find(gamepad_num);
  if (i == _gamepad_axis_map.end()) {
    return Input::get_null_channel();
  }
  std::map<Uint8, unsigned int>::const_iterator j = i->second.find(axis_num);
  if (j == i->second.end()) {
    return Input::get_null_channel();
  }
  return _channels[j->second];
}

const boost::shared_ptr<Channel> GamepadManager::button_channel(Uint8 gamepad_num, Uint8 button_num) const {
  std::map<Uint8, std::map<Uint8, unsigned int> >::const_iterator i = _gamepad_button_map.find(gamepad_num);
  if (i == _gamepad_button_map.end()) {
    return Input::get_null_channel();
  }
  std::map<Uint8, unsigned int>::const_iterator j = i->second.find(button_num);
  if (j == i->second.end()) {
    return Input::get_null_channel();
  }
  return _channels[j->second];
}

bool NullChannel::is_on() const {
  return false;
}

bool NullChannel::matches_frame_events() const {
  return false;
}

float NullChannel::get_value() const {
  return 0.0;
}

bool NullChannel::is_null() const {
  return true;
}

std::string NullChannel::desc(unsigned int desc_type) const {
  return "";
}

KeyChannel::KeyChannel(Keyboard* kbd, SDLKey key) :
  _kbd(kbd),
  _key(key)
{}

bool KeyChannel::is_on() const {
  return _kbd->_sdl_key_state[_key] != _neutral_state;
}

bool KeyChannel::matches_frame_events() const {
  BOOST_FOREACH(const SDL_Event& event, Globals::frame_events) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == _key) {
      return true;
    }
  }
  return false;
}

float KeyChannel::get_value() const {
  return is_on() ? 1.0 : 0.0;
}

void KeyChannel::set_neutral() {
  _neutral_state = _kbd->_sdl_key_state[_key];
}

std::string KeyChannel::desc(unsigned int desc_type) const {
  if (desc_type & CHANNEL_DESC_TYPE_KEYBOARD) {
    std::string upper_name;
    int i = 0;
    BOOST_FOREACH(char n, SDL_GetKeyName(_key)) {
      upper_name.push_back(i == 0 ? std::toupper(int(n)) : std::tolower(int(n)));
      ++i;
    }
    return std::string("[") + upper_name + std::string("]");
  } else {
    return "";
  }
}

MouseButtonChannel::MouseButtonChannel(Mouse* mouse, Uint8 btn) :
  _mouse(mouse),
  _btn(btn)
{}

bool MouseButtonChannel::is_on() const {
  return (_mouse->_btn_mask & SDL_BUTTON(_btn)) != _neutral_state;
}

bool MouseButtonChannel::matches_frame_events() const {
  BOOST_FOREACH(const SDL_Event& event, Globals::frame_events) {
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON(_btn)) {
      return true;
    }
  }
  return false;
}

float MouseButtonChannel::get_value() const {
  return is_on() ? 1.0 : 0.0;
}

void MouseButtonChannel::set_neutral() {
  _neutral_state = _mouse->_btn_mask & SDL_BUTTON(_btn);
}

std::string MouseButtonChannel::desc(unsigned int desc_type) const {
  if (desc_type & CHANNEL_DESC_TYPE_MOUSE) {
    return std::string("MouseBtn") + boost::lexical_cast<std::string>((int)_btn);
  } else {
    return "";
  }
}

MouseMovementChannel::MouseMovementChannel(Mouse* mouse, Uint8 axis) :
  _mouse(mouse),
  _axis(axis)
{}

bool MouseMovementChannel::is_on() const {
  return !similar(get_value(), 0.0);
}

bool MouseMovementChannel::matches_frame_events() const {
  // Could implement this, but can't think of any use for it...
  return false;
}

float MouseMovementChannel::get_value() const {
  int v = (_axis == 0) ? _mouse->_rel_x : _mouse->_rel_y;
  if (v == 0) { return 0.0; }
  float s = Saving::get().config().mouseSensitivity();
  float f = MIN_MOUSE_SENSITIVITY_FULL + s*(MAX_MOUSE_SENSITIVITY_FULL - MIN_MOUSE_SENSITIVITY_FULL);
  return fmax(-1.0, fmin(1.0, v/f));
}

std::string MouseMovementChannel::desc(unsigned int desc_type) const {
  if (desc_type & CHANNEL_DESC_TYPE_MOUSE) {
    if (_axis == 0) {
      return std::string("MouseX");
    } else {
      return std::string("MouseY");
    }
  } else {
    return "";
  }
}

GamepadAxisChannel::GamepadAxisChannel(GamepadManager* gamepad_man, Uint8 gamepad, Uint8 axis) :
  _gamepad_man(gamepad_man),
  _gamepad(gamepad),
  _axis(axis),
  _last_pseudo_frame_event_value(0.0)
{}

float sint16_to_axis_float(Sint16 val) {
  float v = float(val)/32768.0;
  if (v > 1.0 - DEAD_ZONE) {
    return 1.0;
  } else if (v < -1.0 + DEAD_ZONE) {
    return -1.0;
  } else if (v > -DEAD_ZONE && v < DEAD_ZONE) {
    return 0.0;
  } else {
    return v;
  }
}

bool GamepadAxisChannel::is_on() const {
  if (std::fabs(_neutral_value - sint16_to_axis_float(SDL_JoystickGetAxis(_gamepad_man->_gamepads[_gamepad], _axis))) > DEAD_ZONE) {
    return true;
  }
  return false;
}

bool GamepadAxisChannel::matches_frame_events() const {
  // Need to implement this to make it act like a button, and notice when it moves from "off" to "on"
  bool ret = false;
  if (std::fabs(_last_pseudo_frame_event_value) < DEAD_ZONE && std::fabs(get_value()) >= DEAD_ZONE) {
    ret = true;
  }
  
  // Evil use of const_cast, but I can't think of a less evil solution at the moment
  const_cast<GamepadAxisChannel*>(this)->_last_pseudo_frame_event_value = get_value();
  return ret;
}

float GamepadAxisChannel::get_value() const {
  float v = sint16_to_axis_float(SDL_JoystickGetAxis(_gamepad_man->_gamepads[_gamepad], _axis));
  // Treat _neutral_value as though it were 0 on a scale to -1 or to 1 (if v < _neutral_value or v > _neutral_value respectively)
  if (v < _neutral_value) {
    return -((-v) + _neutral_value)/(1 + _neutral_value);
  } else {
    return (v - _neutral_value)/(1 - _neutral_value);
  }
}

void GamepadAxisChannel::set_neutral() {
  _neutral_value = sint16_to_axis_float(SDL_JoystickGetAxis(_gamepad_man->_gamepads[_gamepad], _axis));
}

std::string GamepadAxisChannel::desc(unsigned int desc_type) const {
  // TODO Allow use of human-readable joystick axis names
  if (desc_type & CHANNEL_DESC_TYPE_GAMEPAD) {
    return (boost::format("Gamepad(%u)Axis%u") % (_gamepad+1) % (_axis+1)).str();
  } else {
    return "";
  }
}

GamepadButtonChannel::GamepadButtonChannel(GamepadManager* gamepad_man, Uint8 gamepad, Uint8 button) :
  _gamepad_man(gamepad_man),
  _gamepad(gamepad),
  _button(button)
{}

bool GamepadButtonChannel::is_on() const {
  return SDL_JoystickGetButton(_gamepad_man->_gamepads[_gamepad], _button) != _neutral_state;
}

bool GamepadButtonChannel::matches_frame_events() const {
  BOOST_FOREACH(const SDL_Event& event, Globals::frame_events) {
    if (event.type == SDL_JOYBUTTONDOWN && event.jbutton.which == _gamepad && event.jbutton.button == _button) {
      return true;
    }
  }
  return false;
}

float GamepadButtonChannel::get_value() const {
  if (is_on()) {
    return 1.0;
  }
  return 0.0;
}

void GamepadButtonChannel::set_neutral() {
  _neutral_state = SDL_JoystickGetButton(_gamepad_man->_gamepads[_gamepad], _button);
}

std::string GamepadButtonChannel::desc(unsigned int desc_type) const {
  // TODO Allow use of human-readable joystick button names
  if (desc_type & CHANNEL_DESC_TYPE_GAMEPAD) {
    return (boost::format("Gamepad(%u)Btn%u") % (_gamepad+1) % (_button+1)).str();
  } else {
    return "";
  }
}

InvertAxisChannel::InvertAxisChannel(const boost::shared_ptr<Channel>& chn) :
  _chn(chn)
{}

bool InvertAxisChannel::is_on() const {
  return _chn->is_on();
}

bool InvertAxisChannel::matches_frame_events() const {
  return _chn->matches_frame_events();
}

float InvertAxisChannel::get_value() const {
  return -1*(_chn->get_value());
}

void InvertAxisChannel::set_neutral() {
  _chn->set_neutral();
}

std::string InvertAxisChannel::desc(unsigned int desc_type) const {
  std::string d = _chn->desc(desc_type);
  if (d.size() > 0) {
    return "-" + d;
  } else {
    return "";
  }
}

PseudoAxisChannel::PseudoAxisChannel(const boost::shared_ptr<Channel>& neg, const boost::shared_ptr<Channel>& pos, bool neg_invert, bool pos_invert) :
  _neg(neg),
  _pos(pos),
  _neg_invert(neg_invert),
  _pos_invert(pos_invert)
{}

bool PseudoAxisChannel::is_on() const {
  return _neg->is_on() != _pos->is_on();
}

bool PseudoAxisChannel::matches_frame_events() const {
  return _neg->matches_frame_events() != _pos->matches_frame_events();
}

float PseudoAxisChannel::get_value() const {
  bool neg_on = _neg->is_on();
  bool pos_on = _pos->is_on();
  if (pos_on != neg_on) {
    if (pos_on) {
      return _pos->get_value() * (_pos_invert ? -1 : 1);
    } else {
      return _neg->get_value() * (_neg_invert ? -1 : 1);
    }
  }
  return 0.0;
}

void PseudoAxisChannel::set_neutral() {
  _neg->set_neutral();
  _pos->set_neutral();
}

std::string PseudoAxisChannel::desc(unsigned int desc_type) const {
  std::string n = _neg->desc(desc_type);
  std::string p = _pos->desc(desc_type);
  if (n.size() > 0 && p.size() > 0) {
    return (_neg_invert ? "" : "-") + n + ":" + (_pos_invert ? "-" : "") + p;
  } else {
    return "";
  }
}

PseudoButtonChannel::PseudoButtonChannel(const boost::shared_ptr<Channel>& chn) :
  _chn(chn)
{}

bool PseudoButtonChannel::is_on() const {
  return _chn->is_on();
}

bool PseudoButtonChannel::matches_frame_events() const {
  return _chn->matches_frame_events();
}

float PseudoButtonChannel::get_value() const {
  return std::fabs(_chn->get_value());
}

void PseudoButtonChannel::set_neutral() {
  _chn->set_neutral();
}

std::string PseudoButtonChannel::desc(unsigned int desc_type) const {
  return _chn->desc(desc_type);
}

void MultiChannel::set_neutral() {
  BOOST_FOREACH(boost::shared_ptr<Channel>& ch, _channels) {
    ch->set_neutral();
  }
}

bool MultiOrChannel::is_on() const {
  BOOST_FOREACH(const boost::shared_ptr<Channel>& ch, _channels) {
    if (ch->is_on()) {
      return true;
    }
  }
  return false;
}

bool MultiOrChannel::matches_frame_events() const {
  BOOST_FOREACH(const boost::shared_ptr<Channel>& ch, _channels) {
    if (ch->matches_frame_events()) {
      return true;
    }
  }
  return false;
}

bool MultiOrChannel::is_partially_on() const {
  BOOST_FOREACH(const boost::shared_ptr<Channel>& ch, _channels) {
    if (ch->is_partially_on()) {
      return true;
    }
  }
  return false;
}

float MultiOrChannel::get_value() const {
  float v;
  bool assigned = false;
  BOOST_FOREACH(const boost::shared_ptr<Channel>& ch, _channels) {
    float cand = ch->get_value();
    if ((!assigned) || std::fabs(cand) > std::fabs(v)) {
      v = cand;
      assigned = true;
    }
  }
  return v;
}

std::string MultiOrChannel::desc(unsigned int desc_type) const {
  std::string ret;
  BOOST_FOREACH(const boost::shared_ptr<Channel>& ch, _channels) {
    std::string subch_desc = ch->desc(desc_type);
    if (subch_desc.size() > 0) {
      if (ret.size() > 0) {
        ret += "/";
      }
      ret += subch_desc;
    }
  }
  return ret;
}

bool MultiAndChannel::is_on() const {
  BOOST_FOREACH(const boost::shared_ptr<Channel>& ch, _channels) {
    if (!ch->is_on()) {
      return false;
    }
  }
  return true;
}

bool MultiAndChannel::matches_frame_events() const {
  BOOST_FOREACH(const boost::shared_ptr<Channel>& ch, _channels) {
    if (!ch->matches_frame_events()) {
      return false;
    }
  }
  return true;
}

bool MultiAndChannel::is_partially_on() const {
  BOOST_FOREACH(const boost::shared_ptr<Channel>& ch, _channels) {
    if (ch->is_partially_on()) {
      return true;
    }
  }
  return false;
}

float MultiAndChannel::get_value() const {
  float v;
  bool assigned = false;
  BOOST_FOREACH(const boost::shared_ptr<Channel>& ch, _channels) {
    float cand = ch->get_value();
    if ((!assigned) or std::fabs(cand) < std::fabs(v)) {
      assigned = true;
      v = cand;
    }
  }
  return v;
}

std::string MultiAndChannel::desc(unsigned int desc_type) const {
  std::string ret;
  BOOST_FOREACH(const boost::shared_ptr<Channel>& ch, _channels) {
    std::string subch_desc = ch->desc(desc_type);
    if (subch_desc.size() > 0) {
      if (ret.size() > 0) {
        ret += "+";
      }
      ret += subch_desc;
    }
  }
  return ret;
}

boost::shared_ptr<Channel> Input::_null_channel;

boost::shared_ptr<Keyboard> Input::_kbd;
boost::shared_ptr<Mouse> Input::_mouse;
boost::shared_ptr<GamepadManager> Input::_gp_man;
std::vector<ChannelSource*> Input::_sources;

std::map<ORSave::AxisBoundAction::value_type, boost::shared_ptr<Channel> > Input::_axis_action_map;
std::map<ORSave::ButtonBoundAction::value_type, boost::shared_ptr<Channel> > Input::_button_action_map;
boost::scoped_ptr<ORSave::PresetListType> Input::_preset_list;

void Input::init() {
  _null_channel = boost::shared_ptr<Channel>(new NullChannel);
  
  _kbd = boost::shared_ptr<Keyboard>(new Keyboard);
  _mouse = boost::shared_ptr<Mouse>(new Mouse);
  _gp_man = boost::shared_ptr<GamepadManager>(new GamepadManager);

  _sources.push_back(&*_kbd);
  _sources.push_back(&*_mouse);
  _sources.push_back(&*_gp_man);

  // Load default settings for any input devices not described in the config
  load_config_presets();

  // Create Channel instances to match the configs, and also create Channels for default mappings (i.e. ForceQuit)
  set_channels_from_config();
  
  SDL_Delay(100); // Without this delay we don't always seem to get useful joystick values
  set_neutral();
}

void Input::deinit() {
  _axis_action_map.clear();
  _button_action_map.clear();
  _preset_list.reset();

  _gp_man.reset();
  _mouse.reset();
  _kbd.reset();
  _null_channel.reset();

  _sources.clear();
}

void Input::update() {
  BOOST_FOREACH(ChannelSource* src, _sources) {
    src->update();
  }
}

void Input::set_neutral() {
  update();
  BOOST_FOREACH(ChannelSource* src, _sources) {
    src->set_neutral();
  }
}

boost::shared_ptr<Channel> Input::xml_to_channel(const ORSave::BoundInputType& i) {
  boost::shared_ptr<Channel> chn;
  
  // TODO: Use Factory for this instead, following the polymorphism-detecting pattern in mission_fsm
  if (typeid(i) == typeid(ORSave::KeyInputType)) {
    chn = _kbd->key_channel(
      SDLKey(static_cast<const ORSave::KeyInputType*>(&i)->key())
    );
  } else if (typeid(i) == typeid(ORSave::MouseButtonInputType)) {
    chn = _mouse->button_channel(
      static_cast<const ORSave::MouseButtonInputType*>(&i)->buttonNum()
    );
  } else if (typeid(i) == typeid(ORSave::MouseMovementInputType)) {
    chn = _mouse->movement_channel(
      static_cast<const ORSave::MouseMovementInputType*>(&i)->axisNum()
    );
  } else if (typeid(i) == typeid(ORSave::GamepadButtonInputType)) {
    chn = _gp_man->button_channel(
      static_cast<const ORSave::GamepadButtonInputType*>(&i)->gamepadNum(),
      static_cast<const ORSave::GamepadButtonInputType*>(&i)->buttonNum()
    );
  } else if (typeid(i) == typeid(ORSave::GamepadAxisInputType)) {
    chn = _gp_man->axis_channel(
      static_cast<const ORSave::GamepadAxisInputType*>(&i)->gamepadNum(),
      static_cast<const ORSave::GamepadAxisInputType*>(&i)->axisNum()
    );
  } else if (typeid(i) == typeid(ORSave::InvertAxisInputType)) {
    chn = boost::shared_ptr<Channel>(new InvertAxisChannel(
      xml_to_channel(static_cast<const ORSave::InvertAxisInputType*>(&i)->axis())
    ));
  } else if (typeid(i) == typeid(ORSave::PseudoButtonInputType)) {
    chn = boost::shared_ptr<Channel>(new PseudoButtonChannel(
      xml_to_channel(static_cast<const ORSave::PseudoButtonInputType*>(&i)->axis())
    ));
  } else if (typeid(i) == typeid(ORSave::PseudoAxisInputType)) {
    chn = boost::shared_ptr<Channel>(new PseudoAxisChannel(
      xml_to_channel(static_cast<const ORSave::PseudoAxisInputType*>(&i)->negative()),
      xml_to_channel(static_cast<const ORSave::PseudoAxisInputType*>(&i)->positive()),
      static_cast<const ORSave::PseudoAxisInputType*>(&i)->negInvert(),
      static_cast<const ORSave::PseudoAxisInputType*>(&i)->posInvert()
    ));
  } else if (typeid(i) == typeid(ORSave::LogicalAndInputType)) {
    chn = boost::shared_ptr<Channel>(new MultiAndChannel());
    BOOST_FOREACH(ORSave::BoundInputType& i, static_cast<const ORSave::LogicalAndInputType*>(&i)->input()) {
      static_cast<MultiAndChannel*>(&*chn)->add_channel(xml_to_channel(i));
    }
  } else {
    throw GameException(std::string("Got unexpected BoundInputType in bindings config: ") + typeid(i).name());
  }
  
  return chn;
}

template<typename A, typename C, typename M> void insert_binding(A action, const C& chn, M& map) {
  if (chn->is_null()) {
    return;
  }
  
  typename M::iterator i(map.find(action));
  if (i == map.end()) {
    boost::shared_ptr<MultiOrChannel> top_chn(new MultiOrChannel);
    top_chn->add_channel(chn);
    map.insert(typename M::value_type(action, top_chn));
  } else {
    (static_cast<MultiOrChannel*>(&*(i->second)))->add_channel(chn);
  }
}

void Input::load_config_presets() {
  // Load the encapsulated presets list
  try {
    std::stringstream ss(CAPSULE_PRESETS_XML);
    ORSave::presets_paggr presets_p;
    xml_schema::document_pimpl doc_p(presets_p.root_parser(), presets_p.root_namespace(), presets_p.root_name(), true);
    presets_p.pre();
    doc_p.parse(ss);
    _preset_list.reset(presets_p.post());
  } catch (const xml_schema::parser_exception& e) {
    throw GameException(std::string("Parsing problem while loading presets data : ") + e.text());
  } catch (const std::exception& e) {
    throw GameException(std::string("Error while loading presets data : ") + e.what());
  }

  bool config_dirty = false;

  try {
    Saving::get_input_device(ORSave::InputDeviceNameType::Keyboard);
  } catch (const NoSuchDeviceException& e) {
    Debug::status_msg("Loading default input configuration for keyboard");
    Saving::get().config().inputDevice().push_back(get_preset("Default Keyboard Mapping").inputDevice()._clone());
    config_dirty = true;
  }

  try {
    Saving::get_input_device(ORSave::InputDeviceNameType::Mouse);
  } catch (const GameException& e) {
    Debug::status_msg("Loading default input configuration for mouse");
    Saving::get().config().inputDevice().push_back(get_preset("Default Mouse Mapping").inputDevice()._clone());
    config_dirty = true;
  }

  if (_gp_man->get_num_gamepads() > 0) {
    try {
      ORSave::ConfigType::inputDevice_iterator gp_input = Saving::get_input_device(ORSave::InputDeviceNameType::Gamepad); // Can throw GameException
      if (gp_input->axis_bind().size() == 0 && gp_input->button_bind().size() == 0) {
        // If a gamepad input config exists but is empty, delete it and search for a new preset
        Saving::get().config().inputDevice().erase(gp_input);
        throw GameException("It was empty");
      }
    } catch (const GameException& e) {
      bool found = false;
      std::string gp_name = _gp_man->get_first_gamepad_name();
      Debug::status_msg("Attempting to find an appropriate input configuration for gamepad: '" + gp_name + "'");
      BOOST_FOREACH(const ORSave::PresetType& preset, _preset_list->preset()) {
        BOOST_FOREACH(const std::string& match_name, preset.deviceMatchString()) {
          if (gp_name.find(match_name) != std::string::npos) {
            Debug::status_msg("Loaded gamepad input configuration: '" + preset.presetName() + "'");
            Saving::get().config().inputDevice().push_back(preset.inputDevice()._clone());
            found = true;
            config_dirty = true;
            break;
          }
        }
        if (found) {
          break;
        }
      }
      if (!found) {
        // No preset found, but insert a blank one so user can configure controls themself
        Debug::status_msg("Unable to find an appropriate gamepad input configuration");
        std::auto_ptr<ORSave::InputDeviceType> input_dev(new ORSave::InputDeviceType);
        input_dev->device(ORSave::InputDeviceNameType::Gamepad);
        Saving::get().config().inputDevice().push_back(input_dev.release());
      }
    }
  }

  if (config_dirty) {
    Saving::save();
  }
}

void Input::set_channels_from_config() {
  _axis_action_map.clear();
  _button_action_map.clear();
  
  // Bind all the channels specified by the config file
  BOOST_FOREACH(const ORSave::InputDeviceType& idev, Saving::get().config().inputDevice()) {
    BOOST_FOREACH(const ORSave::AxisBindType& abind, idev.axis_bind()) {
      insert_binding(abind.action(), xml_to_channel(abind.input()), _axis_action_map);
    }
    BOOST_FOREACH(const ORSave::ButtonBindType& bbind, idev.button_bind()) {
      insert_binding(bbind.action(), xml_to_channel(bbind.input()), _button_action_map);
    }
  }
  
  // Bind channels for the fixed default keyboard mappings
  insert_binding(
    ORSave::AxisBoundAction::UIX,
    boost::shared_ptr<Channel>(  
      new PseudoAxisChannel(
        _kbd->key_channel(SDLK_LEFT),
        _kbd->key_channel(SDLK_RIGHT),
        true,
        false
      )
    ),
    _axis_action_map
  );
  
  insert_binding(
    ORSave::AxisBoundAction::UIY,
    boost::shared_ptr<Channel>(  
      new PseudoAxisChannel(
        _kbd->key_channel(SDLK_UP),
        _kbd->key_channel(SDLK_DOWN),
        true,
        false
      )
    ),
    _axis_action_map
  );
  insert_binding(ORSave::ButtonBoundAction::Confirm, _kbd->key_channel(SDLK_RETURN), _button_action_map);
  insert_binding(ORSave::ButtonBoundAction::Confirm, _kbd->key_channel(SDLK_SPACE), _button_action_map);
  insert_binding(ORSave::ButtonBoundAction::Confirm, _kbd->key_channel(SDLK_KP_ENTER), _button_action_map);
  insert_binding(ORSave::ButtonBoundAction::Cancel, _kbd->key_channel(SDLK_ESCAPE), _button_action_map);
  insert_binding(ORSave::ButtonBoundAction::Pause, _kbd->key_channel(SDLK_ESCAPE), _button_action_map);
  insert_binding(ORSave::ButtonBoundAction::Pause, _kbd->key_channel(SDLK_PAUSE), _button_action_map);
  insert_binding(ORSave::ButtonBoundAction::ResetNeutral, _kbd->key_channel(SDLK_F10), _button_action_map);
  insert_binding(ORSave::ButtonBoundAction::ForceQuit, _kbd->key_channel(SDLK_F4), _button_action_map);
  
  // Bind channels for the fixed default mouse button mappings
  insert_binding(ORSave::ButtonBoundAction::Confirm, _mouse->button_channel(1), _button_action_map);
  
  // Alias gameplay motion bindings to UI movement for convenience
  std::map<ORSave::AxisBoundAction::value_type, boost::shared_ptr<Channel> >::iterator i;
  i = _axis_action_map.find(ORSave::AxisBoundAction::TranslateX);
  if (i != _axis_action_map.end()) { insert_binding(ORSave::AxisBoundAction::UIX, i->second, _axis_action_map); }
  i = _axis_action_map.find(ORSave::AxisBoundAction::TranslateY);
  if (i != _axis_action_map.end()) { insert_binding(ORSave::AxisBoundAction::UIY, i->second, _axis_action_map); }
}

const ORSave::PresetType& Input::get_preset(const std::string& name) {
  BOOST_FOREACH(const ORSave::PresetType& preset, _preset_list->preset()) {
    if (preset.presetName() == name) {
      return preset;
    }
  }
  throw GameException("Unable to load preset named '" + name + "'");
}

const Channel& Input::get_axis_ch(ORSave::AxisBoundAction::value_type action) {
  std::map<ORSave::AxisBoundAction::value_type, boost::shared_ptr<Channel> >::iterator i = _axis_action_map.find(action);
  if (i != _axis_action_map.end()) {
    return *(i->second);
  }
  return *_null_channel;
}

const Channel& Input::get_button_ch(ORSave::ButtonBoundAction::value_type action) {
  std::map<ORSave::ButtonBoundAction::value_type, boost::shared_ptr<Channel> >::iterator i = _button_action_map.find(action);
  if (i != _button_action_map.end()) {
    return *(i->second);
  }
  return *_null_channel;
}
