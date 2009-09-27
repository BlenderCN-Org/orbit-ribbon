#include <iostream>

#include "app.h"
#include "except.h"

int main(int argc, char** argv) {
	try {
		App::init();
		App::run();
	} catch (GameQuitException e) {
		std::cout << "Game quit: " << e.get_msg() << " --- Thanks for playing!" << std::endl;
	} catch (GameException e) {
		std::cout << "FATAL EXCEPTION: " << e.get_msg() << std::endl;
	}

	return 0;
}
