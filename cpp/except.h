#ifndef ORBIT_RIBBON_EXCEPT_H
#define ORBIT_RIBBON_EXCEPT_H

#include <string>

class GameException {
	public:
		GameException(const std::string& msg) : _msg(msg) {}
		std::string get_msg() { return _msg; }
	
	private:
		std::string _msg;
};

class GameQuitException : public GameException {
	public:
		GameQuitException(const std::string& msg) : GameException(msg) {}
};

#endif
