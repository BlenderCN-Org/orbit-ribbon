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

#include <vector>

enum BindAction {
	BA_TRANS_X, BA_TRANS_Y, BA_TRANS_Z,
	BA_ROTATE_X, BA_ROTATE_Y, BA_ROTATE_Z,
	BA_RUN_STANCE,
	BA_PAUSE, BA_FORCE_QUIT, BA_RESET_NEUTRAL,
	BA_UI_CONFIRM, BA_UI_BACK, BA_UI_X, BA_UI_Y
};

class Channel {
};

class NullChannel : public Channel {
};

class KeyChannel : public Channel {
};

class GamepadButtonChannel : public Channel {
};

class GamepadAxisChannel : public Channel {
};

class PseudoAxisChannel : public Channel {
};

class MultiOrChannel : public Channel {
};

class MultiAndChannel : public Channel {
};


class ChannelSource {
	public:
		virtual void update();
		virtual std::vector<Channel> channels() =0;
};

class Keyboard : public ChannelSource {
	public:
		Keyboard();
	
	private:
		std::vector<Channel*> channels; // FIXME : This is where I left off. Do I need a vector of auto_ptrs here?
};

class Gamepad : public ChannelSource {
};

class NullGamepad : public ChannelSource {
};

class GamepadManager {
};

class InputManager {
};

#endif
