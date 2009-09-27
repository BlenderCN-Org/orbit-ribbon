#ifndef ORBIT_RIBBON_APP_H
#define ORBIT_RIBBON_APP_H

#include <string>

class App {
	public:
		static void init();
		static void load_area(const std::string& area_name);
		static void load_mission(const std::string& mission_name);
		
		static void run();
	
	private:
		static void _sim_step();
		static void _draw_frame();
};

#endif
