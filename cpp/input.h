/*
input.h: Header for input-management classes.
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

#ifndef ORBIT_RIBBON_INPUT_H
#define ORBIT_RIBBON_INPUT_H

#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/utility.hpp>
#include <map>
#include <SDL/SDL.h>
#include <string>
#include <vector>

#include "autoxsd/save.h"

class Input;

class Channel : boost::noncopyable {
	public:
		virtual bool is_on() const =0;
		virtual bool is_partially_on() const;
		virtual float get_value() const =0;
		virtual void set_neutral();
		virtual bool is_null() const;
		virtual std::string desc() const =0;
};

class NullChannel : public Channel {
	private:
		NullChannel() {}
		
		friend class Input;
	
	public:
		bool is_on() const;
		float get_value() const;
		bool is_null() const;
		std::string desc() const;
};

class ChannelSource : boost::noncopyable {
	protected:
		boost::ptr_vector<Channel> _channels;
		
	public:
		virtual void update() =0;
		virtual void set_neutral();
		
		boost::ptr_vector<Channel>& channels() { return _channels; }
		const boost::ptr_vector<Channel>& channels() const { return _channels; }
};

class KeyChannel;
class Keyboard : public ChannelSource {
	private:
		friend class Input;
		friend class KeyChannel;
		
		Uint8* _sdl_key_state; // Pointer to an array from SDL which can be indexed by SDLKey
		
		Keyboard();
	
	public:
		void update();
		const Channel& key_channel(SDLKey key) const;
};

class KeyChannel : public Channel {
	private:
		friend class Keyboard;
		
		Keyboard* _kbd;
		SDLKey _key;
		Uint8 _neutral_state;
		
		KeyChannel(Keyboard* kbd, SDLKey key);
	
	public:
		bool is_on() const;
		float get_value() const;
		void set_neutral();
		std::string desc() const;
};

class GamepadAxisChannel;
class GamepadButtonChannel;
class GamepadManager : public ChannelSource {
	private:
		friend class Input;
		friend class GamepadAxisChannel;
		friend class GamepadButtonChannel;
		
		std::vector<SDL_Joystick*> _gamepads;
		
		// Outer key is joystick number, inner key is axis/button number, inner value is index into _channels
		std::map<Uint8, std::map<Uint8, unsigned int> > _gamepad_axis_map;
		std::map<Uint8, std::map<Uint8, unsigned int> > _gamepad_button_map;
		
		GamepadManager();
		// TODO Really should have a dtor that destroys _gamepads, but this would be counterproductive without Input::deinit()
	
	public:
		void update();
		const Channel& axis_channel(Uint8 gamepad_num, Uint8 axis_num) const;
		const Channel& button_channel(Uint8 gamepad_num, Uint8 button_num) const;
};

class GamepadAxisChannel : public Channel {
	private:
		friend class GamepadManager;
		
		GamepadManager* _gamepad_man;
		Uint8 _gamepad;
		Uint8 _axis;
		float _neutral_value;
		
		GamepadAxisChannel(GamepadManager* gamepad_man, Uint8 gamepad, Uint8 axis);
	
	public:
		bool is_on() const;
		float get_value() const;
		void set_neutral();
		std::string desc() const;
};

class GamepadButtonChannel : public Channel {
	private:
		friend class GamepadManager;
		
		GamepadManager* _gamepad_man;
		Uint8 _gamepad;
		Uint8 _button;
		Uint8 _neutral_state;
		
		GamepadButtonChannel(GamepadManager* gamepad_man, Uint8 gamepad, Uint8 button);
	
	public:
		bool is_on() const;
		float get_value() const;
		void set_neutral();
		std::string desc() const;
};

class PseudoAxisChannel : public Channel {
	private:
		Channel* _neg;
		Channel* _pos;
		bool _neg_invert, _pos_invert;
	
	public:
		PseudoAxisChannel(Channel* neg, Channel* pos, bool neg_invert, bool pos_invert);
		
		bool is_on() const;
		float get_value() const;
		void set_neutral();
		std::string desc() const;
};

class PseudoButtonChannel : public Channel {
	private:
		Channel* _chn;
	
	public:
		PseudoButtonChannel(Channel* chn);
		
		bool is_on() const;
		float get_value() const;
		void set_neutral();
		std::string desc() const;
};

class MultiChannel : public Channel {
	protected:
		std::vector<Channel*> _channels;
	
	public:
		void add_channel(Channel* ch) { _channels.push_back(ch); }
		
		bool is_on() const =0;
		bool is_partially_on() const =0;
		float get_value() const =0;
		void set_neutral();
		std::string desc() const =0;
};

class MultiOrChannel : public MultiChannel {
	public:
		bool is_on() const;
		bool is_partially_on() const;
		float get_value() const;
		std::string desc() const;	
};

class MultiAndChannel : public MultiChannel {
	public:
		bool is_on() const;
		bool is_partially_on() const;
		float get_value() const;
		std::string desc() const;	
};

class App;

class Input {
	private:
		static boost::ptr_vector<ChannelSource> _sources;
		static std::map<ORSave::AxisBoundAction::Value, Channel*> _axis_action_map;
		static std::map<ORSave::ButtonBoundAction::Value, Channel*> _button_action_map;
		static boost::scoped_ptr<ORSave::PresetListType> _preset_list;
		
		static void init();
		static void update();
		static void set_neutral();
		
		friend class App;
	
	public:
		static NullChannel null_channel;
		
		static const ORSave::PresetListType& get_preset_list() { return *_preset_list; }
		
		static const Channel& get_axis_ch(ORSave::AxisBoundAction::Value action);
		static const Channel& get_button_ch(ORSave::ButtonBoundAction::Value action);
		
		// TODO Add functions here to (de)serialize the action map, and to bind an action
};

#endif