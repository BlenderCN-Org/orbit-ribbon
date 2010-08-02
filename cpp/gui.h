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
#include <string>

#include "geometry.h"
#include "gloo.h"

const Vector GUI_BOX_BORDER(8, 2); // A box's contents are drawn this number of pixels from the left/right and top/bottom of the box respectively
const int LAYOUT_PADDING = 4; // Number of pixels between adjacent items in a layout widget

bool bbox_contains_point(const Size& bbox_size, const Point& upper_left, const Point& test_pt);

class Widget;

struct WidgetLocation {
  const Widget* widget;
  Point upper_left;
  
  WidgetLocation(const Widget* w, const Point& ul) : widget(w), upper_left(ul) {}
  
  Size get_bbox_size();
};

struct WidgetDrawModeMap;

class Widget {
  public:
    enum DrawMode { WIDGET_PASSIVE, WIDGET_FOCUSED, WIDGET_ACTIVATED };
    
    virtual void draw(const Point& upper_left, const WidgetDrawModeMap& mode_map) const =0;
    virtual bool widget_contains_point(const Point& upper_left, const Point& test_pt) const { return bbox_contains_point(get_bbox_size(), upper_left, test_pt); }
    virtual std::list<WidgetLocation> get_children_locations_locations(const Point& upper_left __attribute__ ((unused))) const { return std::list<WidgetLocation>(); }
    virtual bool is_focusable() const { return false; }
    virtual Size get_bbox_size() const =0;
};

class WidgetDrawModeMap {
  public:
    virtual Widget::DrawMode get_mode(const Widget* widget) const =0;
};

class AlwaysPassiveDrawModeMap : public WidgetDrawModeMap {
  public:
    Widget::DrawMode get_mode(const Widget* widget __attribute__ ((unused))) const { return Widget::WIDGET_PASSIVE; }
};

class UserSizedWidget : public Widget {
  private:
    Size _bbox_size;
    
  public:
    UserSizedWidget(const Size& bbox_size) : _bbox_size(bbox_size) {}
    
    void set_bbox_size(const Size& s) { _bbox_size = s; }
    Size get_bbox_size() const { return _bbox_size; }
};

class NullWidget : public UserSizedWidget {
  public:
    NullWidget(const Size& s = Size()) : UserSizedWidget(s) {}
    void draw(const Point& upper_left __attribute__ ((unused)), const WidgetDrawModeMap& mode_map __attribute__ ((unused))) const {}
};

class BoxWidget : public Widget {
  private:
    boost::shared_ptr<Widget> _child;
    
  public:
    BoxWidget(const boost::shared_ptr<Widget>& child) : _child(child) {}
    
    // Note that this has the wrong specification for Widget's virtual draw, so BoxWidget is abstract
    void draw(const Point& upper_left, float r, float g, float b, float a) const;
    
    Size get_bbox_size() const;
    std::list<WidgetLocation> get_children_locations(const Point& upper_left) const;
    
    void set_child(boost::shared_ptr<Widget> child) { _child = child; }
    const Widget& get_child() const { return *_child; }
};

class ButtonWidget : public BoxWidget {
  private:
    static const float PASSIVE_COLOR[4];
    static const float FOCUSED_COLOR[4];
    static const float ACTIVATED_COLOR[4];
    
  public:
    ButtonWidget(const boost::shared_ptr<Widget>& child) : BoxWidget(child) {}
    
    void draw(const Point& upper_left, const WidgetDrawModeMap& mode_map) const;
    bool is_focusable() const { return true; }
};

class TextWidget : public Widget {
  private:
    float _font_height;
    std::string _msg;
  
  public:
    TextWidget(float height, const std::string& msg) : _font_height(height), _msg(msg) {}
    
    void draw(const Point& upper_left, const WidgetDrawModeMap& mode_map) const;
    Size get_bbox_size() const;
};

class LabelButtonWidget : public ButtonWidget {
  public:
      LabelButtonWidget(const std::string& label) : ButtonWidget(boost::shared_ptr<Widget>(new TextWidget(18, label))) {}
};

class LayoutWidget : public Widget {
  public:
    enum Orientation { WIDGET_HORIZONTAL, WIDGET_VERTICAL };
  
  private:
    Orientation _orientation;
    std::list<boost::shared_ptr<Widget> > _children;
  
  public:
    LayoutWidget(Orientation orientation) : _orientation(orientation) {}
    
    void add_child(const boost::shared_ptr<Widget>& child);
    void draw(const Point& upper_left, const WidgetDrawModeMap& mode_map) const;
    Size get_bbox_size() const;
    std::list<WidgetLocation> get_children_locations(const Point& upper_left) const;
};

class CenterWidget : public UserSizedWidget {
  private:
    boost::shared_ptr<Widget> _child;
  
  public:
    CenterWidget(const Size& bbox_size, const boost::shared_ptr<Widget>& child) : UserSizedWidget(bbox_size), _child(child) {}
    
    void draw(const Point& upper_left, const WidgetDrawModeMap& mode_map) const;
    
    std::list<WidgetLocation> get_children_locations(const Point& upper_left) const;
    
    void set_child(boost::shared_ptr<Widget> child) { _child = child; }
    const Widget& get_child() const { return *_child; }
};

class GUI {
  public:
    static void draw_box(const Point& top_left, const Size& size, float r = 0.0, float g = 0.0, float b = 0.0, float a = 0.5);
};

#endif