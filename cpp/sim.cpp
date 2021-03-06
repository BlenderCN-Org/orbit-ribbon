/*
sim.cpp: Implementation of the Sim class.
This class is responsible for managing the gameplay simulation and running the ODE stepper.

Copyright 2011 David Simon <david.mike.simon@gmail.com>

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

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
//#include <GL/glew.h>
#include <ode/ode.h>
#include <cctype>
#include <vector>

#include "autoxsd/orepkgdesc.h"
#include "constants.h"
#include "debug.h"
#include "gameobj.h"
#include "globals.h"
#include "sim.h"

dWorldID ode_world;
dSpaceID static_space;
dSpaceID dyn_space;
dJointGroupID contact_group;

const unsigned int MAXIMUM_CONTACT_POINTS = 16;

bool SimpleContactHandler::handle_collision(float t __attribute__ ((unused)), dGeomID other __attribute__ ((unused)), const dContactGeom* contacts __attribute__ ((unused)), unsigned int contacts_len __attribute__ ((unused))) {
  return true;
}

void CollisionTracker::Collision::init(float t, dGeomID o, const dContactGeom* c, unsigned int c_len) {
  step_time = t;
  other = o;
  other_gameobj = static_cast<GameObj*>(dGeomGetData(o));
  
  contacts.reserve(c_len);
  for (unsigned int i = 0; i < c_len; ++i) {
    contacts.push_back(*c);
    ++c;
  }
}

CollisionTracker::CollisionTracker() {
  _collisions.reset(new std::vector<CollisionTracker::Collision>);
}

bool CollisionTracker::handle_collision(float t, dGeomID o, const dContactGeom* c, unsigned int c_len) {
  _collisions->push_back(Collision());
  _collisions->back().init(t, o, c, c_len);
  return should_contact(t, o, c, c_len);
}

std::auto_ptr<std::vector<CollisionTracker::Collision> > CollisionTracker::get_collisions() {
  std::auto_ptr<std::vector<CollisionTracker::Collision> > ret(_collisions);
  _collisions.reset(new std::vector<CollisionTracker::Collision>);
  return ret;
}

dWorldID Sim::get_ode_world() {
  return ode_world;
}

dSpaceID Sim::get_static_space() {
  return static_space;
}

dSpaceID Sim::get_dyn_space() {
  return dyn_space;
}

void Sim::init() {
  dInitODE();
  ode_world = dWorldCreate();
  dWorldSetQuickStepNumIterations(ode_world, 10);
  static_space = dHashSpaceCreate(0);
  dyn_space = dHashSpaceCreate(0);
  contact_group = dJointGroupCreate(0);
}

void Sim::deinit() {
  dJointGroupDestroy(contact_group);
  dSpaceDestroy(dyn_space);
  dSpaceDestroy(static_space);
  dWorldDestroy(ode_world);
  dCloseODE();
}

void collision_callback(void* data, dGeomID o1, dGeomID o2) {
  float step_time = *static_cast<float*>(data);
  
  static dContactGeom contacts[MAXIMUM_CONTACT_POINTS];
  if (dGeomIsSpace(o1) or dGeomIsSpace(o2)) {
    dSpaceCollide2(o1, o2, NULL, &collision_callback);
  } else {
    if (dGeomIsEnabled(o1) && dGeomIsEnabled(o2)) {
      unsigned int len = dCollide(o1, o2, MAXIMUM_CONTACT_POINTS, &(contacts[0]), sizeof(dContactGeom));
      if (len > 0) {
        /*
        for (unsigned int i = 0; i < len; ++i) {
          Debug::debug_msg("C" + boost::lexical_cast<std::string>(i) + " " + boost::lexical_cast<std::string>(contacts[i].side1) + ":" + boost::lexical_cast<std::string>(contacts[i].side2));
        }
        */
        CollisionHandler* o1h = static_cast<CollisionHandler*>(dGeomGetData(o1));
        CollisionHandler* o2h = static_cast<CollisionHandler*>(dGeomGetData(o2));
        bool contact1 = o1h->handle_collision(step_time, o2, &(contacts[0]), len);
        for (unsigned int i = 0; i < len; ++i) {
          // Reverse each contact so that o2h sees itself as the first geom
          contacts[i].normal[0] = -contacts[i].normal[0];
          contacts[i].normal[1] = -contacts[i].normal[1];
          contacts[i].normal[2] = -contacts[i].normal[2];
          dGeomID tempG = contacts[i].g1; contacts[i].g1 = contacts[i].g2; contacts[i].g2 = tempG;
          int tempS = contacts[i].side1; contacts[i].side1 = contacts[i].side2; contacts[i].side2 = tempS;
        }
        bool contact2 = o2h->handle_collision(step_time, o1, &(contacts[0]), len);
        for (unsigned int i = 0; i < len; ++i) {
          // Reverse each contact so that o2h sees itself as the first geom
          contacts[i].normal[0] = -contacts[i].normal[0];
          contacts[i].normal[1] = -contacts[i].normal[1];
          contacts[i].normal[2] = -contacts[i].normal[2];
          dGeomID tempG = contacts[i].g1; contacts[i].g1 = contacts[i].g2; contacts[i].g2 = tempG;
          int tempS = contacts[i].side1; contacts[i].side1 = contacts[i].side2; contacts[i].side2 = tempS;
        }
        if (contact1 && contact2) {
          dContact contact;
          contact.surface.mode = dContactApprox1 | dContactBounce;
          contact.surface.bounce = 0.5;
          contact.surface.mu = 5000;
          for (unsigned int ci = 0; ci < len; ++ci) {
            contact.geom = contacts[ci];
            dJointID joint = dJointCreateContact(ode_world, contact_group, &contact);
            dJointAttach(joint, dGeomGetBody(o1), dGeomGetBody(o2));
          }
        }
      }
    }
  }
}

void Sim::sim_step() {
  // Check for collisions
  dJointGroupEmpty(contact_group);
  float step_time = (float)Globals::total_steps;
  dSpaceCollide(dyn_space, &step_time, &collision_callback); // Collisions among dyn_space objects
  dSpaceCollide2(dGeomID(dyn_space), dGeomID(static_space), &step_time, &collision_callback); // Collisions between dyn_space objects and static_space objects
  
  // Run the simulation
  dWorldQuickStep(ode_world, 1.0f/MAX_FPS);
  
  // Have each GameObj do whatever it needs to do each step (including damping)
  for (GOMap::iterator i = Globals::gameobjs.begin(); i != Globals::gameobjs.end(); ++i) {
    i->second->step();
  }
  
  Globals::total_steps += 1;
}

std::auto_ptr<OdeEntity> Sim::gen_empty_body() {
  return std::auto_ptr<OdeEntity>(new OdeEntity);
}

std::auto_ptr<OdeEntity> Sim::gen_sphere_body(float mass, float rad) {
  dBodyID body = dBodyCreate(ode_world);
  dMass ode_mass;
  dMassSetSphereTotal(&ode_mass, mass, rad);
  dBodySetMass(body, &ode_mass);
  return std::auto_ptr<OdeEntity>(new OdeEntity(body));
}

dBodyID OdeEntity::get_id() {
  if (_id == 0) {
    throw GameException("Attempted to retrieve id from bodyless OdeEntity");
  } else {
    return _id;
  }
}

dGeomID OdeEntity::get_geom(const std::string& gname) {
  std::map<std::string, dGeomID>::const_iterator i = _geoms.find(gname);
  if (i == _geoms.end()) {
    throw GameException("Unable to retrieve geom named " + gname);
  } else {
    return i->second;
  }
}

CollisionHandler* OdeEntity::get_geom_ch(const std::string& gname) {
  return static_cast<CollisionHandler*>(dGeomGetData(get_geom(gname)));
}

void OdeEntity::set_geom(const std::string& gname, dGeomID geom, std::auto_ptr<CollisionHandler> ch, const ORE1::ObjType* offset) {
  // From here on out, we have to manage the memory for this CollisionHandler manually
  dGeomSetData(geom, ch.release());
  
  // TODO possible optimization : to automatically use a space if a body has more than one geom
  // This could be done without changing the external set_geom/get_geom interface
  std::map<std::string, dGeomID>::iterator i = _geoms.find(gname);
  if (i != _geoms.end()) {
    delete (CollisionHandler*)(dGeomGetData(i->second));
    dGeomDestroy(i->second);
    _geoms.erase(i);
  }
  
  if (geom != 0) {
    if (_id != 0) {
      // This automatically sets the geom's position and orientation to the body's
      dGeomSetBody(geom, _id);
    } else {
      dGeomSetPosition(geom, _last_pos.x, _last_pos.y, _last_pos.z);
      dMatrix3 matr;
      OdeGeomUtil::gen_ode_rot_matr(_last_rot, matr);
      dGeomSetRotation(geom, matr);
    }
    _geoms[gname] = geom;

    if (offset != 0) {
      if (_id != 0) {
        dGeomSetOffsetPosition(geom, offset->pos()[0], offset->pos()[1], offset->pos()[2]);
        
        dMatrix3 rot;
        OdeGeomUtil::gen_ode_rot_matr(offset->rot(), rot);
        dGeomSetOffsetRotation(geom, rot);
      } else {
        Point p(dGeomGetPosition(geom));
        p += Point(offset->pos()[0], offset->pos()[1], offset->pos()[2]);
        dGeomSetPosition(geom, p[0], p[1], p[2]);

        // TODO Also update rotation by the offset amount
      }
    }
  }
}

void OdeEntity::set_pos(const Point& pos) {
  _last_pos = pos;
  
  if (_id != 0) {
    dBodySetPosition(_id, pos.x, pos.y, pos.z);
  } else {
    BOOST_FOREACH(GeomMap::value_type& p, _geoms) {
      dGeomSetPosition(p.second, pos.x, pos.y, pos.z);
    }
  }
}

void OdeEntity::set_rot(const boost::array<float, 9>& rot) {
  _last_rot = rot;
  
  // Convert column-major 3x3 to row-major 3x4
  dMatrix3 matr;
  OdeGeomUtil::gen_ode_rot_matr(rot, matr);
  if (_id != 0) {
    dBodySetRotation(_id, matr);
  } else {
    BOOST_FOREACH(GeomMap::value_type& p, _geoms) {
      dGeomSetRotation(p.second, matr);
    }
  }
}

GameObj* OdeEntity::get_gameobj_from_body(dBodyID b) {
  return static_cast<GameObj*>(dBodyGetData(b));
}

void OdeEntity::set_gameobj(GameObj* g) {
  if (_id != 0) {
    dBodySetData(_id, g);
  }
}

OdeEntity::~OdeEntity() {
  if (_id != 0) {
    dBodyDestroy(_id);
  }
  
  BOOST_FOREACH(GeomMap::value_type& p, _geoms) {
    delete (CollisionHandler*)(dGeomGetData(p.second));
    dGeomDestroy(p.second);
  }
}
