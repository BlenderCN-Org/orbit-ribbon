/*
gameobj.h: Header of the GameObj class
GameObj is the base class for in-game objects, and handles ODE bodies and geoms.

Copyright 2009 David Simon. You can reach me at david.mike.simon@gmail.com

This file is part of Orbit Ribbon.

Orbit Ribbon is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Orbit Ribbon is distributed in the hope that it will be awesome,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Orbit Ribbon.  If not, see http://www.gnu.org/licenses/
*/

#ifndef ORBIT_RIBBON_GAMEOBJ_H
#define ORBIT_RIBBON_GAMEOBJ_H

#include <GL/glew.h>
#include <GL/gl.h>
#include <ode/ode.h>
#include <boost/array.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include <utility>

#include "except.h"
#include "geometry.h"

namespace ORE1 { class ObjType; }

class GameObj : boost::noncopyable {
	public:
		GameObj(const ORE1::ObjType& obj);
		virtual ~GameObj();
		
		const Point& get_pos() const { return _pos; }
		void set_pos(const Point& pos);
		
		const boost::array<GLfloat, 9>& get_rot() const { return _rot; }
		void set_rot(const boost::array<GLfloat, 9>& rot);
		
		const Vector& get_vel() const { return _vel; }
		
		std::string to_str() const;
		
		void draw(bool near);
		void step();
	
	protected:
		dBodyID get_body() const { return _body; }
		void set_body(dBodyID body);
		
		dGeomID get_geom() const { return _geom; }
		void set_geom(dGeomID geom);
		
		virtual void near_draw_impl() {}
		virtual void far_draw_impl() { near_draw_impl(); }
		virtual void step_impl() {}
	
	private:
		Point _pos;
		boost::array<GLfloat, 9> _rot; // 3x3 column-major
		Vector _vel;
		
		// Damping coefficients for linear and angular velocity along each body axis
		// For example, vel_damp_coef[0] is the linear damping coefficient along relative x axis
		// Each step, vel_damp_coef[0]/MAX_FPS * relative x velocity is removed
		GLfloat _vel_damp_coef[3];
		GLfloat _ang_damp_coef[3];
		
		dBodyID _body;
		dGeomID _geom;
};

class GOFactory {
	public:
		virtual boost::shared_ptr<GameObj> create(const ORE1::ObjType& obj) =0;
};

class GOFactoryRegistry {
	private:
		// Since we can't rely on static object ctors being called before any GOAutoRegistrations are initialized...
		// We'll just create simple free store allocation (which we can safely not worry about deleting) on first call to register_*
		
		struct _Factories {
			std::map<std::string, boost::shared_ptr<GOFactory> > _factory_map;
			boost::shared_ptr<GOFactory> _default_factory;
		};
		static bool _initialized;
		static _Factories* _factories;
		
		static void init() {
			if (!_initialized) {
				_factories = new _Factories;
				_initialized = true;
			}
		}
	
	public:
		static void register_factory(const std::string& name, const boost::shared_ptr<GOFactory>& factory) {
			init();
			std::map<std::string, boost::shared_ptr<GOFactory> >::iterator i = _factories->_factory_map.find(name);
			if (i != _factories->_factory_map.end()) {
				throw GameException("Attempted to register over existing GameObject factory for " + name);
			}
			_factories->_factory_map.insert(std::pair<const std::string, boost::shared_ptr<GOFactory> >(name, factory));
		}
		
		static void register_default_factory(const boost::shared_ptr<GOFactory>& factory) { 
			init();
			if (_factories->_default_factory) {
				throw GameException("Attempted to register over existing default GameObject factory");
			}
			_factories->_default_factory = factory; 
		}
		
		static boost::shared_ptr<GameObj> create(const ORE1::ObjType& obj);
};

template<typename T> class _GOAutoFactory : public GOFactory {
	boost::shared_ptr<GameObj> create(const ORE1::ObjType& obj) {
		return boost::shared_ptr<GameObj>(new T(obj));
	}
};

template<typename T> class GOAutoRegistration {
	public:
		GOAutoRegistration() {
			GOFactoryRegistry::register_default_factory(boost::shared_ptr<GOFactory>(new _GOAutoFactory<T>));
		}
		
		GOAutoRegistration(const std::string& name) {
			GOFactoryRegistry::register_factory(name, boost::shared_ptr<GOFactory>(new _GOAutoFactory<T>));
		}
};

#endif