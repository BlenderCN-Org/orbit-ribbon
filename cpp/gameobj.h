#ifndef ORBIT_RIBBON_GAMEOBJ_H
#define ORBIT_RIBBON_GAMEOBJ_H

#include <GL/gl.h>
#include <ode/ode.h>

#include "geometry.h"

class GameObj {
	public:
		static GLfloat get_default_vel_damp_coef() { return 0.15; }
		static GLfloat get_default_ang_damp_coef() { return 0.15; }
		
		GameObj(const Point& npos, const Vector& nrot);
		~GameObj();
		
		const Point& get_pos() const { return pos; }
		void set_pos(const Point& npos);
		
		const Vector& get_rot() const { return rot; }
		void set_rot(const Vector& nrot);
		
		const Vector& get_vel() const { return vel; }
		
		std::string to_str() const;

		void near_draw();
		void far_draw();
		void step();
		void damp();
	
	protected:
		dBodyID get_body() const { return body; }
		void set_body(dBodyID nbody);

		dGeomID get_geom() const { return geom; }
		void set_geom(dGeomID ngeom);
		
		virtual void near_draw_impl() {}
		virtual void far_draw_impl() { near_draw_impl(); }
		virtual void step_impl() {}
	
	private:
		Point pos;
		Vector rot;
		Vector vel;
		GLfloat vel_damp_coef;
		GLfloat ang_damp_coef;
		
		dBodyID body;
		dGeomID geom;
};

#endif
