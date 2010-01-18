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
#include <cmath>

#include "constants.h"
#include "input.h"

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
	BOOST_FOREACH(Channel& ch, _channels) {
		ch.set_neutral();
	}
}

Keyboard::Keyboard() {
	update();
	for (unsigned int i = (const unsigned int)SDLK_FIRST; i < (const unsigned int)SDLK_LAST; ++i) {
		_channels.push_back(new KeyChannel(this, SDLKey(i)));
	}
}

void Keyboard::update() {
	_sdl_key_state = SDL_GetKeyState(NULL);
}

const Channel& Keyboard::key_channel(SDLKey key) const {
	return _channels[(unsigned int)key];
}

GamepadManager::GamepadManager() {
	for (Uint8 i = 0; i < SDL_NumJoysticks(); ++i) {
		SDL_Joystick* gp = SDL_JoystickOpen(i);
		_gamepads.push_back(gp);
		for (Uint8 a = 0; a < SDL_JoystickNumAxes(gp); ++a) {
			_gamepad_axis_map[i][a] = _channels.size();
			_channels.push_back(new GamepadAxisChannel(this, i, a));
		}
		for (Uint8 b = 0; b < SDL_JoystickNumButtons(gp); ++b) {
			_gamepad_button_map[i][b] = _channels.size();
			_channels.push_back(new GamepadButtonChannel(this, i, b));
		}
	}
	update();
}

void GamepadManager::update() {
	SDL_JoystickUpdate();
}

const Channel& GamepadManager::axis_channel(Uint8 gamepad_num, Uint8 axis_num) const {
	std::map<Uint8, std::map<Uint8, unsigned int> >::const_iterator i = _gamepad_axis_map.find(gamepad_num);
	if (i == _gamepad_axis_map.end()) {
		return Input::null_channel;
	}
	std::map<Uint8, unsigned int>::const_iterator j = i->second.find(axis_num);
	if (j == i->second.end()) {
		return Input::null_channel;
	}
	return _channels[j->second];
}

const Channel& GamepadManager::button_channel(Uint8 gamepad_num, Uint8 button_num) const {
	std::map<Uint8, std::map<Uint8, unsigned int> >::const_iterator i = _gamepad_button_map.find(gamepad_num);
	if (i == _gamepad_button_map.end()) {
		return Input::null_channel;
	}
	std::map<Uint8, unsigned int>::const_iterator j = i->second.find(button_num);
	if (j == i->second.end()) {
		return Input::null_channel;
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
	return std::string("Key:") + SDL_GetKeyName(_key);
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
	if (v > 1.0) {
		return 1.0;
	} else if (v < -1.0) {
		return -1.0;
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

PseudoAxisChannel::PseudoAxisChannel(Channel* neg, Channel* pos) :
	_neg(neg),
	_pos(pos)
{}

bool PseudoAxisChannel::is_on() const {
	return _neg->is_on() != _pos->is_on();
}

float PseudoAxisChannel::get_value() const {
	bool neg_on = _neg->is_on();
	bool pos_on = _pos->is_on();
	if (pos_on != neg_on) {
		if (pos_on) {
			return _pos->get_value();
		} else {
			return -_neg->get_value();
		}
	}
	return 0;
}

void PseudoAxisChannel::set_neutral() {
	_neg->set_neutral();
	_pos->set_neutral();
}

std::string PseudoAxisChannel::desc() const {
	return _neg->desc() + "-" + _pos->desc();
}

void MultiChannel::set_neutral() {
	BOOST_FOREACH(Channel* ch, _channels) {
		ch->set_neutral();
	}
}

bool MultiOrChannel::is_on() const {
	BOOST_FOREACH(Channel* ch, _channels) {
		if (ch->is_on()) {
			return true;
		}
	}
	return false;
}

bool MultiOrChannel::is_partially_on() const {
	BOOST_FOREACH(Channel* ch, _channels) {
		if (ch->is_partially_on()) {
			return true;
		}
	}
	return false;
}

float MultiOrChannel::get_value() const {
	float v;
	BOOST_FOREACH(Channel* ch, _channels) {
		float cand = ch->get_value();
		if (std::fabs(cand) > std::fabs(v)) {
			v = cand;
		}
	}
	return v;
}

std::string MultiOrChannel::desc() const {
	std::string ret;
	BOOST_FOREACH(Channel* ch, _channels) {
		if (ret.size() > 0) {
			ret += "/";
		}
		ret += ch->desc();
	}
	return ret;
}

bool MultiAndChannel::is_on() const {
	BOOST_FOREACH(Channel* ch, _channels) {
		if (!ch->is_on()) {
			return false;
		}
	}
	return true;
}

bool MultiAndChannel::is_partially_on() const {
	BOOST_FOREACH(Channel* ch, _channels) {
		if (ch->is_partially_on()) {
			return true;
		}
	}
	return false;
}

float MultiAndChannel::get_value() const {
	float v;
	bool assigned = false;
	BOOST_FOREACH(Channel* ch, _channels) {
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
	BOOST_FOREACH(Channel* ch, _channels) {
		if (ret.size() > 0) {
			ret += "+";
		}
		ret += ch->desc();
	}
	return ret;
}

boost::ptr_vector<ChannelSource> Input::_sources;
std::map<ORSave::BoundAction::Value, Channel*> Input::_action_map;

void Input::init() {
	_sources.push_back(new Keyboard);
	_sources.push_back(new GamepadManager);
}

void Input::update() {
	BOOST_FOREACH(ChannelSource& src, _sources) {
		src.update();
	}
}

void Input::set_neutral() {
	BOOST_FOREACH(ChannelSource& src, _sources) {
		src.set_neutral();
	}
}

NullChannel Input::null_channel;

const Channel& Input::get_ch(ORSave::BoundAction::Value action) {
	std::map<ORSave::BoundAction::Value, Channel*>::iterator i = _action_map.find(action);
	if (i != _action_map.end()) {
		return *(i->second);
	}
	return null_channel;
}