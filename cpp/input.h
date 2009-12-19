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
