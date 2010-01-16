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

#include "constants.h"
#include "input.h"

// How far from -1, 0, or 1 where we consider an input axis to just be at those exact values
const float DEAD_ZONE = 0.001;

// Space separated list of key names to ignore in the input module because they cause problems
const char* const IGNORE_KEYS = "[-] numlock";

bool Channel::is_partially_on() const {
	return is_on();
}

void Channel::set_neutral() {
	// Do nothing by default.
}

Keyboard::Keyboard() {
}

void Keyboard::update() {
}

void Keyboard::set_neutral() {
}

const Channel* Keyboard::key_channel(SDLKey key) const {
}

GamepadManager::GamepadManager() {
}

void GamepadManager::update() {
}

void GamepadManager::set_neutral() {
}

const Channel* GamepadManager::axis_channel(Uint8 gamepad_num, Uint8 axis_num) const {
}

const Channel* GamepadManager::button_channel(Uint8 gamepad_num, Uint8 button_num) const {
}

bool NullChannel::is_on() const {
	return false;
}

float NullChannel::get_value() const {
	return 0.0;
}

bool NullChannel::matches_events(const std::vector<SDL_Event>& events) const {
	return false;
}

std::string NullChannel::desc() const {
	return std::string("NULL");
}

KeyChannel::KeyChannel(Keyboard* kbd, SDLKey key) :
	_kbd(kbd),
	_key(key)
{
}

bool KeyChannel::is_on() const {
}

float KeyChannel::get_value() const {
}

bool KeyChannel::matches_events(const std::vector<SDL_Event>& events) const {
}

std::string KeyChannel::desc() const {
}

GamepadAxisChannel::GamepadAxisChannel(GamepadManager* gamepad_man, Uint8 axis) :
	_gamepad_man(gamepad_man),
	_axis(axis)
{
}

bool GamepadAxisChannel::is_on() const {
}

float GamepadAxisChannel::get_value() const {
}

bool GamepadAxisChannel::matches_events(const std::vector<SDL_Event>& events) const {
}

void GamepadAxisChannel::set_neutral() {
}

std::string GamepadAxisChannel::desc() const {
}

GamepadButtonChannel::GamepadButtonChannel(GamepadManager* gamepad_man, Uint8 btn) :
	_gamepad_man(gamepad_man),
	_btn(btn)
{
}

bool GamepadButtonChannel::is_on() const {
}

float GamepadButtonChannel::get_value() const {
}

bool GamepadButtonChannel::matches_events(const std::vector<SDL_Event>& events) const {
}

std::string GamepadButtonChannel::desc() const {
}

PseudoAxisChannel::PseudoAxisChannel(Channel* neg, Channel* pos) {
}

bool PseudoAxisChannel::is_on() const {
}

float PseudoAxisChannel::get_value() const {
}

bool PseudoAxisChannel::matches_events(const std::vector<SDL_Event>& events) const {
}

void PseudoAxisChannel::set_neutral() {
}

std::string PseudoAxisChannel::desc() const {
}

void MultiChannel::set_neutral() {
}

bool MultiOrChannel::is_on() const {
}

bool MultiOrChannel::is_partially_on() const {
}

float MultiOrChannel::get_value() const {
}

bool MultiOrChannel::matches_events(const std::vector<SDL_Event>& events) const {
}

std::string MultiOrChannel::desc() const {
}

bool MultiAndChannel::is_on() const {
}

bool MultiAndChannel::is_partially_on() const {
}

float MultiAndChannel::get_value() const {
}

bool MultiAndChannel::matches_events(const std::vector<SDL_Event>& events) const {
}

std::string MultiAndChannel::desc() const {
}

boost::scoped_ptr<Keyboard> Input::_keyboard;
boost::scoped_ptr<GamepadManager> Input::_gamepad_man;
std::map<BindAction, Channel*> Input::_action_map;

void Input::init() {
}

void Input::update() {
}

void Input::set_neutral() {
}

NullChannel Input::null_channel;

const Channel* Input::get_channel(BindAction action) {
}