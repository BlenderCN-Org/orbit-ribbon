/*
avatar.h: Header of the Avatar class
Avatar is a GameObject representing the player character

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

#ifndef ORBIT_RIBBON_AVATAR_H
#define ORBIT_RIBBON_AVATAR_H

#include <ode/ode.h>
#include <boost/shared_ptr.hpp>

#include "gameobj.h"
#include "geometry.h"
#include "sim.h"

// Maximum amount per second that each of these avatar-relative values can be changed for
// aligning with a running surface, and maximum delta for attachment
const float RUNNING_ADJ_RATE_X_ROT = 0.5;   const float RUNNING_MAX_DELTA_X_ROT = 0.8; // Radians
const float RUNNING_ADJ_RATE_Z_ROT = 0.3;   const float RUNNING_MAX_DELTA_Z_ROT = 0.6; // Radians
const float RUNNING_ADJ_RATE_Y_POS = 6.0;   const float RUNNING_MAX_DELTA_Y_POS = 0.5;  // Meters
const float RUNNING_ADJ_RATE_Y_LVEL = 100.0;  const float RUNNING_MAX_DELTA_Y_LVEL = 7.0; // Meters per second
const float RUNNING_ADJ_RATE_X_AVEL = 5.0;  const float RUNNING_MAX_DELTA_X_AVEL = 7.0; // Radians per second
const float RUNNING_ADJ_RATE_Z_AVEL = 5.0;  const float RUNNING_MAX_DELTA_Z_AVEL = 7.0; // Radians per second

class MeshAnimation;
namespace ORE1 { class ObjType; }

class AvatarGameObj;
class AvatarGameObj : public GameObj {
  private:
    // Tracking how far off we are from ideal running state, if attached
    // These are kept so they can be used for debugging by GameplayMode
    Vector _sn;
    float _xrot_delta;
    float _zrot_delta;
    float _ypos_delta;
    float _ylvel_delta;
    float _xavel_delta;
    float _zavel_delta;
    unsigned int _norm_coll_steptime;
    unsigned int _run_coll_steptime;
    
    boost::shared_ptr<MeshAnimation> _anim_fly_to_prerun;
    float _uprightness;
    
    float _height; // Distance from top of head to bottom of feet
    float _coll_rad; // Radius of the collision capsule
    bool _attached;
    bool _attached_this_frame;
    
    void update_geom_offsets();
    bool check_attachment(float yp_delta, const Vector& sn);
    
    class AvatarContactHandler;
    friend class AvatarContactHandler;
    class AvatarContactHandler : public SimpleContactHandler {
      private:
        AvatarGameObj* _avatar;
        
      public:
        AvatarContactHandler(AvatarGameObj* avatar) : _avatar(avatar) {}
        bool handle_collision(float t, dGeomID o, const dContactGeom* c, unsigned int c_len);
    };
    
    class StickyAttachmentContactHandler : public SimpleContactHandler {
      private:
        AvatarGameObj* _avatar;
        
      public:
        StickyAttachmentContactHandler(AvatarGameObj* avatar) : _avatar(avatar) {}
        bool handle_collision(float t, dGeomID o, const dContactGeom* c, unsigned int c_len);
    };
    
  protected:
    void step_impl();
    void near_draw_impl();
  
  public:
    AvatarGameObj(const ORE1::ObjType& obj);
    
    float get_last_xrot() { return _xrot_delta; }
    float get_last_zrot() { return _zrot_delta; }
    float get_last_ypos() { return _ypos_delta; }
    float get_last_xavel() { return _xavel_delta; }
    float get_last_zavel() { return _zavel_delta; }
    float get_last_ylvel() { return _ylvel_delta; }
    bool is_attached() { return _attached; }
    unsigned int get_last_norm_coll_age();
    unsigned int get_last_run_coll_age();
};

#endif
