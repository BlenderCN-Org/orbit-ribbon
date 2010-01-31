/*
input.cpp: Implementation of input-management classes.
These classes are responsible for taking input from the keyboard and gamepad, and binding input to game actions.

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
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <cctype>
#include <cmath>
#include <sstream>
#include <typeinfo>

#include "autoxsd/presets.xml.h"
#include "autoxsd/save.h"
#include "constants.h"
#include "debug.h"
#include "except.h"
#include "input.h"
#include "saving.h"

// How far from -1, 0, or 1 where we consider an input axis to just be at those exact values
const float DEAD_ZONE = 0.001;

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

void GamepadManager::update() {
	SDL_JoystickUpdate();
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

float NullChannel::get_value() const {
	return 0.0;
}

bool NullChannel::is_null() const {
	return true;
}

std::string NullChannel::desc() const {
	return std::string("NULL");
}

KeyChannel::KeyChannel(Keyboard* kbd, SDLKey key) :
	_kbd(kbd),
	_key(key)
{
	set_neutral();
}

bool KeyChannel::is_on() const {
	return _kbd->_sdl_key_state[_key] != _neutral_state;
}

float KeyChannel::get_value() const {
	if (is_on()) {
		return 1.0;
	}
	return 0.0;
}

void KeyChannel::set_neutral() {
	_neutral_state = _kbd->_sdl_key_state[_key];
}

std::string KeyChannel::desc() const {
	std::string upper_name;
	BOOST_FOREACH(char n, SDL_GetKeyName(_key)) {
		upper_name.push_back(std::toupper(int(n)));
	}
	return std::string("[") + upper_name + std::string("]");
}

GamepadAxisChannel::GamepadAxisChannel(GamepadManager* gamepad_man, Uint8 gamepad, Uint8 axis) :
	_gamepad_man(gamepad_man),
	_gamepad(gamepad),
	_axis(axis)
{
	set_neutral();
}

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

std::string GamepadAxisChannel::desc() const {
	// TODO Allow use of human-readable joystick axis names
	return (boost::format("Gamepad(%u)Axis:%u") % (_gamepad+1) % (_axis+1)).str();
}

GamepadButtonChannel::GamepadButtonChannel(GamepadManager* gamepad_man, Uint8 gamepad,  Uint8 button) :
	_gamepad_man(gamepad_man),
	_gamepad(gamepad),
	_button(button)
{
	set_neutral();
}

bool GamepadButtonChannel::is_on() const {
	return SDL_JoystickGetButton(_gamepad_man->_gamepads[_gamepad], _button) != _neutral_state;
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

std::string GamepadButtonChannel::desc() const {
	// TODO Allow use of human-readable joystick button names
	return (boost::format("Gamepad(%u)Btn:%u") % (_gamepad+1) % (_button+1)).str();
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

std::string PseudoAxisChannel::desc() const {
	return _neg->desc() + "-" + _pos->desc();
}

PseudoButtonChannel::PseudoButtonChannel(const boost::shared_ptr<Channel>& chn) :
	_chn(chn)
{}

bool PseudoButtonChannel::is_on() const {
	return _chn->is_on();
}

float PseudoButtonChannel::get_value() const {
	return std::fabs(_chn->get_value());
}

void PseudoButtonChannel::set_neutral() {
	_chn->set_neutral();
}

std::string PseudoButtonChannel::desc() const {
	return _chn->desc();
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
	BOOST_FOREACH(const boost::shared_ptr<Channel>& ch, _channels) {
		float cand = ch->get_value();
		if (std::fabs(cand) > std::fabs(v)) {
			v = cand;
		}
	}
	return v;
}

std::string MultiOrChannel::desc() const {
	std::string ret;
	BOOST_FOREACH(const boost::shared_ptr<Channel>& ch, _channels) {
		if (ret.size() > 0) {
			ret += "/";
		}
		ret += ch->desc();
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

std::string MultiAndChannel::desc() const {
	std::string ret;
	BOOST_FOREACH(const boost::shared_ptr<Channel>& ch, _channels) {
		if (ret.size() > 0) {
			ret += "+";
		}
		ret += ch->desc();
	}
	return ret;
}

boost::shared_ptr<Channel> Input::_null_channel;

boost::shared_ptr<Keyboard> Input::_kbd;
boost::shared_ptr<GamepadManager> Input::_gp_man;
std::vector<ChannelSource*> Input::_sources;

std::map<ORSave::AxisBoundAction::Value, boost::shared_ptr<Channel> > Input::_axis_action_map;
std::map<ORSave::ButtonBoundAction::Value, boost::shared_ptr<Channel> > Input::_button_action_map;
boost::scoped_ptr<ORSave::PresetListType> Input::_preset_list;


void Input::init() {
	_null_channel = boost::shared_ptr<Channel>(new NullChannel);
	
	// Load the encapsulated presets list
	std::stringstream ss(CAPSULE_PRESETS_XML);
	_preset_list.reset(ORSave::presets(ss, xsd::cxx::tree::flags::dont_validate).release());
	
	bool config_dirty = false;
	
	// Set up the keyboard input source. If no keyboard input config exists, create one from the default preset.
	_kbd = boost::shared_ptr<Keyboard>(new Keyboard);
	_sources.push_back(&*_kbd);
	try {
		Saving::get_input_device(ORSave::InputDeviceNameType::Keyboard);
	} catch (const GameException& e) {
		Debug::status_msg("Loading default input configuration for keyboard");
		Saving::get().config().inputDevice().push_back(get_preset("Default Keyboard Mapping").inputDevice());
		config_dirty = true;
	}
	
	// Set up any gamepads. TODO: If any gamepads were detected, but no gamepad input config exists, try to find an appropriate preset
	_gp_man = boost::shared_ptr<GamepadManager>(new GamepadManager);
	_sources.push_back(&*_gp_man);
	
	// Create Channel instances to match the configs
	set_channels_from_config();
	
	if (config_dirty) {
		Saving::save();
	}
}

void Input::update() {
	BOOST_FOREACH(ChannelSource* src, _sources) {
		src->update();
	}
}

void Input::set_neutral() {
	BOOST_FOREACH(ChannelSource* src, _sources) {
		src->set_neutral();
	}
}

boost::shared_ptr<Channel> Input::xml_to_channel(const ORSave::BoundInputType& i) {
	boost::shared_ptr<Channel> chn;
	
	// Oi, this is bad use of polymorphism, but there's no other good way I can see to do it
	if (typeid(i) == typeid(ORSave::KeyInputType)) {
		chn = _kbd->key_channel(
			SDLKey(static_cast<const ORSave::KeyInputType*>(&i)->key())
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

void Input::set_channels_from_config() {
	_axis_action_map.clear();
	_button_action_map.clear();
	
	// Create all the channels specified by the config file
	BOOST_FOREACH(const ORSave::InputDeviceType& idev, Saving::get().config().inputDevice()) {
		BOOST_FOREACH(const ORSave::AxisBindType& abind, idev.axis_bind()) {
			insert_binding(abind.action(), xml_to_channel(abind.input()), _axis_action_map);
		}
		BOOST_FOREACH(const ORSave::ButtonBindType& bbind, idev.button_bind()) {
			insert_binding(bbind.action(), xml_to_channel(bbind.input()), _button_action_map);
		}
	}
	
	// Create channels for the fixed default keyboard mappings
	
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
				_kbd->key_channel(SDLK_DOWN),
				_kbd->key_channel(SDLK_UP),
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
	
	insert_binding(ORSave::ButtonBoundAction::ResetNeutral, _kbd->key_channel(SDLK_F10), _button_action_map);
	
	insert_binding(ORSave::ButtonBoundAction::ForceQuit, _kbd->key_channel(SDLK_F4), _button_action_map);
}

const ORSave::PresetType& Input::get_preset(const std::string& name) {
	BOOST_FOREACH(const ORSave::PresetType& preset, _preset_list->preset()) {
		if (preset.presetName() == name) {
			return preset;
		}
	}
	throw GameException("Unable to load preset named '" + name + "'");
}

const Channel& Input::get_axis_ch(ORSave::AxisBoundAction::Value action) {
	std::map<ORSave::AxisBoundAction::Value, boost::shared_ptr<Channel> >::iterator i = _axis_action_map.find(action);
	if (i != _axis_action_map.end()) {
		return *(i->second);
	}
	return *_null_channel;
}

const Channel& Input::get_button_ch(ORSave::ButtonBoundAction::Value action) {
	std::map<ORSave::ButtonBoundAction::Value, boost::shared_ptr<Channel> >::iterator i = _button_action_map.find(action);
	if (i != _button_action_map.end()) {
		return *(i->second);
	}
	return *_null_channel;
}