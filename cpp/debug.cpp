#include <string>
#include <iostream>

#include "debug.h"

void Debug::error_msg(const std::string& msg) {
	std::cout << "ERROR: " << msg << std::endl;
}
