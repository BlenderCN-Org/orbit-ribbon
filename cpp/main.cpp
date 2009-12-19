#include <list>
#include <string>

#include "app.h"

int main(int argc, char** argv) {
	std::list<std::string> args;
	for (int i = 0; i < argc; ++i) {
		args.push_back(std::string(argv[i]));
	}
	
	App::run(args);
	return 0;
}
