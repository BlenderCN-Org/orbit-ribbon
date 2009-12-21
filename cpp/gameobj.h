#ifndef ORBIT_RIBBON_GAMEOBJ_H
#define ORBIT_RIBBON_GAMEOBJ_H

#include <GL/gl.h>
#include <ode/ode.h>
#include <boost/array.hpp>

#include "geometry.h"

class GameObj {
	public:
		GameObj(const Point& npos, const boost::array<GLfloat, 9>& nrot);
		~GameObj();
		
		const Point& get_pos() const { return pos; }
		void set_pos(const Point& npos);
		
		const boost::array<GLfloat, 9>& get_rot() const { return rot; }
		void set_rot(const boost::array<GLfloat, 9>& nrot);
		
		const Vector& get_vel() const { return vel; }
		
		std::string to_str() const;

		void draw(bool near);
		void step();
	
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
		boost::array<GLfloat, 9> rot; // 3x3 column-major
		Vector vel;

		// Damping coefficients for linear and angular velocity along each body axis
		// For example, vel_damp_coef[0] is the linear damping coefficient along relative x axis
		// Each step, vel_damp_coef[0]/MAX_FPS * relative x velocity is removed
		GLfloat vel_damp_coef[3];
		GLfloat ang_damp_coef[3];
		
		dBodyID body;
		dGeomID geom;
};

#endif
