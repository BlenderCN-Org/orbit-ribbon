#include <iostream>

#include "app.h"
#include "except.h"

int main(int argc, char** argv) {
	try {
		App::init();
	} catch (GameException e) {
		std::cout << "FATAL EXCEPTION DURING INIT: " << e.get_msg() << std::endl;
	}

	try {
		App::run();
	} catch (GameQuitException e) {
		std::cout << "Game quit: " << e.get_msg() << " --- Thanks for playing!" << std::endl;
	} catch (GameException e) {
		std::cout << "FATAL EXCEPTION DURING RUN: " << e.get_msg() << std::endl;
	}
	
	return 0;
}
