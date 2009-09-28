#ifndef ORBIT_RIBBON_SIM_H
#define ORBIT_RIBBON_SIM_H

#include <ode/ode.h>

class App;

class Sim {
	public:
		static dWorldID get_ode_world();
		static dSpaceID get_static_space();
		static dSpaceID get_dyn_space();
	
	private:
		static void _init();
		static void _sim_step();
		friend class App;
};

#endif
