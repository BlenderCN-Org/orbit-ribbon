#ifndef ORBIT_RIBBON_APP_H
#define ORBIT_RIBBON_APP_H

#include <GL/gl.h>

#include <string>
#include <vector>

class App {
	public:
		static void run(const std::vector<std::string>& arguments);
		
	private:
		static void frame_loop();
};

#endif
