/*
gui.h: Header for GUI related classes and functions.

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

#ifndef ORBIT_RIBBON_GUI_H
#define ORBIT_RIBBON_GUI_H

#include <boost/shared_ptr.hpp>
#include <vector>

#include "geometry.h"
#include "gloo.h"

const Vector GUI_BOX_BORDER(8, 2); // A box's contents are drawn this number of pixels from the left/right and top/bottom of the box respectively
const int LAYOUT_PADDING = 4; // Number of pixels between adjacent items in a layout widget

class Widget {
	public:
		enum DrawMode { WIDGET_PASSIVE, WIDGET_FOCUSED, WIDGET_ACTIVATED };
		
		virtual void draw(const Point& upper_left, DrawMode mode) const =0;
		virtual const Widget* widget_containing_point(const Point& widget_upper_left, const Point& test_pt) const;
		virtual Size get_size() const =0;
};

class NullWidget : public Widget {
	private:
		Size _size;
		
	public:
		NullWidget(const Size& size = Size()) : _size(size) {}
		void draw(const Point& upper_left, Widget::DrawMode mode) const {}
		Size get_size() const { return _size; }
};

class BoxWidget : public Widget {
	private:
		boost::shared_ptr<Widget> _child;
		float _r, _g, _b, _a;
		
	public:
		BoxWidget(boost::shared_ptr<Widget> child, float r = 0.0, float g = 0.0, float b = 0.0, float a = 0.5)
			: _child(child), _r(r), _g(g), _b(b), _a(a) {}
		
		void draw(const Point& upper_left, Widget::DrawMode mode) const;
		Widget* widget_containing_point(const Point& widget_upper_left, const Point& test_pt) const;
		Size get_size() const;
		
		void set_box_colors(float r, float g, float b) { _r = r; _g = g; _b = b; }
		void set_box_alpha(float a) { _a = a; }
		
		const Widget& get_child() const { return *_child; }
		void set_child(boost::shared_ptr<Widget> child) { _child = child; }
};

class LayoutWidget : public Widget {
	public:
		enum Orientation { WIDGET_HORIZONTAL, WIDGET_VERTICAL };
	
	private:
		Orientation _orientation;
	
	public:
		std::vector<boost::shared_ptr<Widget> > children;
		
		LayoutWidget(Orientation orientation) : _orientation(orientation) {}
		
		void draw(const Point& upper_left, Widget::DrawMode mode) const;
		Size get_size() const;
};

// TODO: Also going to need a TableLayoutWidget sooner or later

class GUI {
	public:
		static void draw_box(const Point& top_left, const Size& size, float r = 0.0, float g = 0.0, float b = 0.0, float a = 0.5);
};

#endif