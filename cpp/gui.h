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
#include <list>

#include "geometry.h"
#include "gloo.h"

const Vector GUI_BOX_BORDER(8, 2); // A box's contents are drawn this number of pixels from the left/right and top/bottom of the box respectively
const int LAYOUT_PADDING = 4; // Number of pixels between adjacent items in a layout widget

class Widget;
struct WidgetLocation {
	const Widget* widget;
	Point upper_left;
	Size bbox_size;
	
	WidgetLocation(const Widget* w, const Point& ul, const Size& bbs) : widget(w), upper_left(ul), bbox_size(bbs) {}
};

class Widget {
	public:
		enum DrawMode { WIDGET_PASSIVE, WIDGET_FOCUSED, WIDGET_ACTIVATED };
		
		virtual void draw(const Point& upper_left, DrawMode mode) const =0;
		virtual bool widget_contains_point(const Point& upper_left, const Point& test_pt) const { return bbox_contains_point(upper_left, test_pt); }
		virtual std::list<WidgetLocation> get_focusable_children(const Point& upper_left __attribute__ ((unused))) const { return std::list<WidgetLocation>(); }
		virtual bool is_focusable() const { return false; }
		virtual Size get_bbox_size() const =0;
		
		bool bbox_contains_point(const Point& upper_left, const Point& test_pt) const;
};

class UserSizedWidget : public Widget {
	private:
		Size _bbox_size;
		
	public:
		UserSizedWidget(const Size& bbox_size) : _bbox_size(bbox_size) {}
		
		void set_bbox_size(const Size& s) { _bbox_size = s; }
		virtual Size get_bbox_size() const { return _bbox_size; }
};

class NullWidget : public UserSizedWidget {
	public:
		NullWidget(const Size& s = Size()) : UserSizedWidget(s) {}
		void draw(const Point& upper_left __attribute__ ((unused)), Widget::DrawMode mode __attribute__ ((unused))) const {}
};

class BoxWidget : public Widget {
	private:
		boost::shared_ptr<Widget> _child;
		float _r, _g, _b, _a;
		
	public:
		BoxWidget(const boost::shared_ptr<Widget>& child, float r = 0.0, float g = 0.0, float b = 0.0, float a = 0.5)
			: _child(child), _r(r), _g(g), _b(b), _a(a) {}
		
		void draw(const Point& upper_left, Widget::DrawMode mode) const;
		bool widget_contains_point(const Point& test_pt) const;
		Size get_bbox_size() const;
		std::list<WidgetLocation> get_focusable_children(const Point& upper_left) const;
		
		void set_box_color(float r, float g, float b) { _r = r; _g = g; _b = b; }
		void set_box_alpha(float a) { _a = a; }
		
		void set_child(boost::shared_ptr<Widget> child) { _child = child; }
		const Widget& get_child() const { return *_child; }
};

class ButtonWidget : public BoxWidget {
	public:
		ButtonWidget(const boost::shared_ptr<Widget>& child);
		
		void draw(const Point& upper_left, Widget::DrawMode mode) const;
		bool is_focusable() const { return true; }
};

class LayoutWidget : public Widget {
	public:
		enum Orientation { WIDGET_HORIZONTAL, WIDGET_VERTICAL };
	
	private:
		Orientation _orientation;
		std::list<boost::shared_ptr<Widget> > children;
	
	public:
		LayoutWidget(Orientation orientation) : _orientation(orientation) {}
		
		void add_child(const boost::shared_ptr<Widget>& child);
		void draw(const Point& upper_left, Widget::DrawMode mode) const;
		Size get_bbox_size() const;
		std::list<WidgetLocation> get_focusable_children(const Point& upper_left) const;
};

class TextWidget : public Widget {
	private:
		float _font_height;
		std::string _msg;
	
	public:
		TextWidget(float height, const std::string& msg) : _font_height(height), _msg(msg) {}
		
		void draw(const Point& upper_left, Widget::DrawMode mode) const;
		Size get_bbox_size() const;
};

// TODO: Also going to need a TableLayoutWidget sooner or later

class GUI {
	public:
		static void draw_box(const Point& top_left, const Size& size, float r = 0.0, float g = 0.0, float b = 0.0, float a = 0.5);
};

#endif