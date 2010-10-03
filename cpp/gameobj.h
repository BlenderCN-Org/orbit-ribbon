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
#include <boost/scoped_ptr.hpp>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "factory.h"
#include "geometry.h"
#include "sim.h"

namespace ORE1 { class ObjType; }

class GameObj : boost::noncopyable {
  public:
    GameObj(const Point& pos, std::auto_ptr<OdeEntity> entity = Sim::gen_empty_body());
    GameObj(const ORE1::ObjType& obj, std::auto_ptr<OdeEntity> entity = Sim::gen_empty_body());
    
    const Point& get_pos() const { return _pos; }
    void set_pos(const Point& pos);
    
    const boost::array<float, 9>& get_rot() const { return _rot; }
    void set_rot(const boost::array<float, 9>& rot);
    
    const Vector& get_vel() const { return _vel; }
    float get_speed() const { return _vel.mag(); }
    
    std::string to_str() const;
    
    void draw(bool near);
    void step();
    
    Point get_rel_point_pos(const Point& p) const;
    Point get_pos_rel_point(const Point& p) const;
    Vector vector_to_world(const Vector& v) const;
    Vector vector_from_world(const Vector& v) const;
  
  protected:
    OdeEntity& get_entity() { return *_entity; }
    const OdeEntity& get_entity() const { return *_entity; }

    const ORE1::ObjType& get_libscene_obj(const std::string& name) const;
    
    virtual void near_draw_impl() {}
    virtual void far_draw_impl() { near_draw_impl(); }
    virtual void step_impl() {}
  
  private:
    Point _pos;
    boost::array<float, 9> _rot; // 3x3 column-major
    Vector _vel;
    boost::scoped_ptr<CollisionHandler> _coll_handler;
    std::map<std::string, const ORE1::ObjType&> _scene_objs;
    
    // Damping coefficients for linear and angular velocity along each body axis
    // For example, vel_damp_coef[0] is the linear damping coefficient along relative x axis
    // Each step, vel_damp_coef[0]/MAX_FPS * relative x velocity is removed
    float _vel_damp_coef[3];
    float _ang_damp_coef[3];
    
    std::auto_ptr<OdeEntity> _entity;

    void common_setup();
};

class GameObjFactorySpec : public FactorySpecBase<GameObj, ORE1::ObjType> {
  public:
    std::string extract_name(const ORE1::ObjType& source);
};

#endif
