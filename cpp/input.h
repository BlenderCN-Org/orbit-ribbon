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

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>
#include <map>
#include <SDL/SDL.h>
#include <string>
#include <vector>

enum BindAction {
	BA_TRANS_X, BA_TRANS_Y, BA_TRANS_Z,
	BA_ROTATE_X, BA_ROTATE_Y, BA_ROTATE_Z,
	BA_RUN_STANCE,
	BA_PAUSE, BA_FORCE_QUIT, BA_RESET_NEUTRAL,
	BA_UI_CONFIRM, BA_UI_BACK, BA_UI_X, BA_UI_Y
};

class Channel : boost::noncopyable {
	public:
		virtual bool is_on() const =0;
		virtual bool is_partially_on() const;
		virtual float get_value() const =0;
		virtual bool matches_events(const std::vector<SDL_Event>& events) const =0;
		virtual void set_neutral();
		virtual std::string desc() const =0;
};

class ChannelSource : boost::noncopyable {
	protected:
		boost::ptr_vector<Channel> _channels;
		
	public:
		virtual void update() =0;
		virtual void set_neutral() =0;
		
		boost::ptr_vector<Channel>& channels() { return _channels; }
		const boost::ptr_vector<Channel>& channels() const { return _channels; }
};

class Input;

class Keyboard : public ChannelSource {
	private:
		friend class Input;
	
		Keyboard();
	
	public:
		void update();
		void set_neutral();
		const Channel* key_channel(SDLKey key) const;
};

class GamepadManager : public ChannelSource {
	private:
		friend class Input;
		
		std::map<Uint8, SDL_Joystick*> _gamepads;
		
		// Outer key is joystick number, inner key is axis/button number, inner value is index into _channels
		std::map<Uint8, std::map<Uint8, unsigned int> > _gamepad_axis_map;
		std::map<Uint8, std::map<Uint8, unsigned int> > _gamepad_button_map;
		
		GamepadManager();
	
	public:
		void update();
		void set_neutral();
		const Channel* axis_channel(Uint8 gamepad_num, Uint8 axis_num) const;
		const Channel* button_channel(Uint8 gamepad_num, Uint8 button_num) const;
};

class NullChannel : public Channel {
	public:
		bool is_on() const;
		float get_value() const;
		bool matches_events(const std::vector<SDL_Event>& events) const;
		std::string desc() const;
};

class KeyChannel : public Channel {
	private:
		friend class Keyboard;
		
		Keyboard* _kbd;
		SDLKey _key;
		
		KeyChannel(Keyboard* kbd, SDLKey key);
	
	public:
		bool is_on() const;
		float get_value() const;
		bool matches_events(const std::vector<SDL_Event>& events) const;
		std::string desc() const;
};

class GamepadAxisChannel : public Channel {
	private:
		friend class Gamepad;
		
		GamepadManager* _gamepad_man;
		Uint8 _axis;
		
		GamepadAxisChannel(GamepadManager* gamepad_man, Uint8 axis);
	
	public:
		bool is_on() const;
		float get_value() const;
		bool matches_events(const std::vector<SDL_Event>& events) const;
		void set_neutral();
		std::string desc() const;
};

class GamepadButtonChannel : public Channel {
	private:
		friend class Gamepad;
		
		GamepadManager* _gamepad_man;
		Uint8 _btn;
		
		GamepadButtonChannel(GamepadManager* gamepad_man, Uint8 btn);
	
	public:
		bool is_on() const;
		float get_value() const;
		bool matches_events(const std::vector<SDL_Event>& events) const;
		std::string desc() const;
};

class PseudoAxisChannel : public Channel {
	private:
		Channel* _neg;
		Channel* _pos;
	
	public:
		PseudoAxisChannel(Channel* neg, Channel* pos);
		
		bool is_on() const;
		float get_value() const;
		bool matches_events(const std::vector<SDL_Event>& events) const;
		void set_neutral();
		std::string desc() const;
};

class MultiChannel : public Channel {
	private:
		std::vector<Channel*> _channels;
	
	public:
		void add_channel(Channel* ch) { _channels.push_back(ch); }
		
		bool is_on() const =0;
		bool is_partially_on() const =0;
		float get_value() const =0;
		bool matches_events(const std::vector<SDL_Event>& events) const =0;
		void set_neutral();
		std::string desc() const =0;
};

class MultiOrChannel : public MultiChannel {
	public:
		bool is_on() const;
		bool is_partially_on() const;
		float get_value() const;
		bool matches_events(const std::vector<SDL_Event>& events) const;
		std::string desc() const;	
};

class MultiAndChannel : public MultiChannel {
	public:
		bool is_on() const;
		bool is_partially_on() const;
		float get_value() const;
		bool matches_events(const std::vector<SDL_Event>& events) const;
		std::string desc() const;	
};

class App;

class Input {
	private:
		static boost::scoped_ptr<Keyboard> _keyboard;
		static boost::scoped_ptr<GamepadManager> _gamepad_man;
		
		static std::map<BindAction, Channel*> _action_map;
		
		static void init();
		static void update();
		static void set_neutral();
		
		friend class App;
	
	public:
		static NullChannel null_channel;
		
		static const Channel* get_channel(BindAction action);
		
		// TODO Add functions here to (de)serialize the action map, and to bind an action
};

#endif