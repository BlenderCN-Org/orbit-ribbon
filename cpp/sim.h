/*
sim.h: Header of the Sim class.
This class is responsible for managing the gameplay simulation and running the ODE stepper.

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

#ifndef ORBIT_RIBBON_SIM_H
#define ORBIT_RIBBON_SIM_H

#include <boost/array.hpp>
#include <boost/utility.hpp>
#include <memory>
#include <vector>
#include <ode/ode.h>

#include "geometry.h"

class Sim;
class App;
class GameObj;
class CollisionTracker;

class CollisionHandler {
  public:
    // Returns true if a contact joint should be created (a joint is actually created only if both geoms' handlers agree that one should be)
    virtual bool handle_collision(float t, dGeomID o, const dContactGeom* c, unsigned int c_len) =0;
};

class SimpleContactHandler : public CollisionHandler {
  public:
    SimpleContactHandler() {}
    
    bool handle_collision(float t, dGeomID other, const dContactGeom* contacts, unsigned int contacts_len);
};

class CollisionTracker : public CollisionHandler {
  public:
    class Collision {
      public:
        float step_time;
        dGeomID other;
        const GameObj* other_gameobj;
        std::vector<dContactGeom> contacts;
      
      private:
        friend class CollisionTracker;
      
        Collision() {}
        void init(float t, dGeomID o, const dContactGeom* c, unsigned int c_len);
    };
    
    CollisionTracker();
    bool handle_collision(float t, dGeomID o, const dContactGeom* c, unsigned int c_len);
    bool has_collisions() const { return _collisions->size() > 0; }
    std::auto_ptr<std::vector<Collision> > get_collisions();
    
    virtual bool should_contact(float t, dGeomID o, const dContactGeom* c, unsigned int c_len) const =0;
  
  private:
    std::auto_ptr<std::vector<Collision> > _collisions;
};


class OdeEntity;
class Sim {
  public:
    static dWorldID get_ode_world();
    static dSpaceID get_static_space();
    static dSpaceID get_dyn_space();
    
    static std::auto_ptr<OdeEntity> gen_empty_body();
    static std::auto_ptr<OdeEntity> gen_sphere_body(float mass, float rad);
    
    static void sim_step();
  
  private:
    static void init();
    friend class App;
};

class OdeEntity : boost::noncopyable {
  private:
    friend class Sim;
    
    typedef std::map<std::string, dGeomID> GeomMap;
    
    // These are used when _id isn't set to fill in starting position of newly inserted geoms
    Point _last_pos;
    boost::array<float, 9> _last_rot;
    
    dBodyID _id;
    GeomMap _geoms;
    
    OdeEntity() : _id(0) {}
    OdeEntity(dBodyID id) : _id(id) {}
    
  public:
    bool has_id() const { return _id != 0; }
    dBodyID get_id();
    
    dGeomID get_geom(const std::string& gname);
    CollisionHandler* get_geom_ch(const std::string& gname);
    void set_geom(const std::string& gname, dGeomID geom, std::auto_ptr<CollisionHandler> ch);
    
    void set_pos(const Point& pos);
    void set_rot(const boost::array<float, 9>& rot);
    
    static GameObj* get_gameobj_from_body(dBodyID b);
    void set_gameobj(GameObj* g);
    
    ~OdeEntity();
};

class OdeGeomUtil {
  public:
    static Point get_rel_point_pos(dGeomID g, const Point& p) {
      dVector3 res;
      dGeomGetRelPointPos(g, p.x, p.y, p.z, res);
      return Point(res);
    }
    
    static Point get_pos_rel_point(dGeomID g, const Point& p) {
      dVector3 res;
      dGeomGetPosRelPoint(g, p.x, p.y, p.z, res);
      return Point(res);
    }
    
    static Vector vector_to_world(dGeomID g, const Vector& v) {
      dVector3 res;
      dGeomVectorToWorld(g, v.x, v.y, v.z, res);
      return Vector(res);
    }
    
    static Vector vector_from_world(dGeomID g, const Vector& v) {
      dVector3 res;
      dGeomVectorFromWorld(g, v.x, v.y, v.z, res);
      return Vector(res);
    }
};  

#endif
