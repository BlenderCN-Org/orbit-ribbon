#include <GL/gl.h>
#include <ode/ode.h>

#include "gameobj.h"
#include "geometry.h"

GameObj::GameObj(const Point& npos, const Rotation& nrot) :
	pos(npos),
	rot(nrot),
	vel(Vector()),
	vel_damp_coef(get_default_vel_damp_coef()),
	ang_damp_coef(get_default_ang_damp_coef()),
	body(0),
	geom(0)
{}

GameObj::~GameObj() {
	set_body(0);
	set_geom(0);
}

void GameObj::set_pos(const Point& npos) {
}

void GameObj::set_rot(const Rotation& nrot) {
}

std::string GameObj::to_str() const {
	// FIXME : Implement
	return std::string("");
}

void GameObj::near_draw() {
}

void GameObj::far_draw() {
}

void GameObj::step() {
	step_impl();
}

void GameObj::damp() {
}

void GameObj::set_body(dBodyID nbody) {
}

void GameObj::set_geom(dGeomID ngeom) {
}
