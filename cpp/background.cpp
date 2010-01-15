/*
background.cpp: Implementation of the Background class
The Background class is responsible for outside lighting and for drawing the Smoke Ring and other distant background objects

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

#include <cmath>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "autoxsd/orepkgdesc.h"
#include "background.h"
#include "geometry.h"
#include "gloo.h"

SkySettings::SkySettings() :
	ring_angle(0),
	ring_y_offset(0),
	ring_d_offset(0),
	tilt_angle(0),
	tilt_x(0),
	tilt_z(0),
	t3_angle(0)
{}

SkySettings::SkySettings(const boost::array<float, 7>& args) :
	ring_angle(args[0]),
	ring_y_offset(args[1]),
	ring_d_offset(args[2]),
	tilt_angle(args[3]),
	tilt_x(args[4]),
	tilt_z(args[5]),
	t3_angle(args[6])
{}

SkySettings::SkySettings(const ORE1::SkySettingsType& area) :
	ring_angle(area.ringAngle()),
	ring_y_offset(area.ringYOffset()),
	ring_d_offset(area.ringDOffset()),
	tilt_angle(area.tiltAngle()),
	tilt_x(area.tiltX()),
	tilt_z(area.tiltZ()),
	t3_angle(area.t3Angle())
{}

void SkySettings::fill_array(boost::array<float, 7>& tgt) {
	tgt[0] = ring_angle;
	tgt[1] = ring_y_offset;
	tgt[2] = ring_d_offset;
	tgt[3] = tilt_angle;
	tgt[4] = tilt_x;
	tgt[5] = tilt_z;
	tgt[6] = t3_angle;
}

Point Background::get_game_origin() {
	const float d = GOLD_DIST + _sky.ring_d_offset;
	return -Point(d*std::sin(-rev2rad(_sky.ring_angle)), _sky.ring_y_offset, d*std::cos(-rev2rad(_sky.ring_angle)));
}

Point Background::convert_to_sky_coords(const Point& pt) {
	//TODO Implement
}

Background::Background(const SkySettings& sky) {
	set_sky(sky);
}

void Background::set_sky(const SkySettings& sky) {
	_sky = sky;
	
	// Generate the transformation matrix that moves us from the game origin to Voy
	// TODO Probably more efficient to do this without using OpenGL, but this sure is convenient
	{
		GLOOPushedMatrix pm;
		glLoadIdentity();
		glRotatef(_sky.tilt_angle, _sky.tilt_x, 0, _sky.tilt_z); // Apply tilt
		glTranslatef(0.0, -_sky.ring_y_offset, GOLD_DIST + _sky.ring_d_offset); // Move out to Voy
		glRotatef(rev2deg(_sky.ring_angle), 0, 1, 0); // Rotate the Smoke Ring around Voy
		glGetFloatv(GL_MODELVIEW_MATRIX, _skyMatr.begin());
	}
}

float Background::get_dist_from_ring(const Point& pt) {
	Point local = convert_to_sky_coords(pt);
	float xDist = std::abs(pt.dist_to(Point(0,0,0) - GOLD_DIST));
	float yDist = local.y;
	return std::sqrt(xDist*xDist + yDist*yDist);
}

void Background::set_clear_color() {
	// TODO Have this change based on atmospheric thickness
	glClearColor(0.6, 0.6, 1.0, 0.0);
}

void Background::draw() {
	GLOOPushedMatrix pm;
	glMultMatrixf(_skyMatr.begin());
	
	// TODO Draw stars if atmospheric thickness is low enough
	
	// TODO Draw the Smoke Ring and the gas torus in a way that allows us to transition between inside and outside
	
	// Draw and set up lighting for T3
	// TODO Draw T3 itself
	// TODO If I feel nerdy one day, I should verify that this makes T3 spin in the right direction as t3_angle increases
	float t3_pos[4] = {std::sin(-rev2rad(_sky.t3_angle))*T3_DIST, 0.0, std::cos(-rev2rad(_sky.t3_angle))*T3_DIST, 1.0};
	glLightfv(GL_LIGHT1, GL_POSITION, t3_pos);
	
	// TODO Draw and set up lighting for Voy
	
	// Set up ambient lighting (so that areas not lit by T3 or Voy aren't completely dark)
	float part_ald = std::sqrt(2)*AMB_LIGHT_DIST;
	float pos3[4] = {0.0, AMB_LIGHT_DIST, 0.0, 1.0};
	glLightfv(GL_LIGHT3, GL_POSITION, pos3);
	float pos4[4] = {part_ald, -part_ald, 0.0, 1.0};
	glLightfv(GL_LIGHT4, GL_POSITION, pos4);
	float pos5[4] = {-0.5*part_ald, -part_ald, 0.866*part_ald, 1.0};
	glLightfv(GL_LIGHT5, GL_POSITION, pos5);
	float pos6[4] = {-0.5*part_ald, -part_ald, -0.866*part_ald, 1.0};
	glLightfv(GL_LIGHT6, GL_POSITION, pos6);
	
	// TODO Draw clouds
	
	// TODO Draw distant Smoke Ring megaflora
}