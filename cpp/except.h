/*
except.h: Header for exceptions used by Orbit Ribbon

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

#ifndef ORBIT_RIBBON_EXCEPT_H
#define ORBIT_RIBBON_EXCEPT_H

#include <string>
#include <exception>

class GameException : public std::exception {
  public:
    GameException(const std::string& msg) : _msg(msg) {}
    virtual ~GameException() throw() {}
    std::string get_msg() const throw() { return _msg; }
    const char* what() const throw() { return _msg.c_str(); }
  
  private:
    std::string _msg;
};

class GameQuitException : public GameException {
  public:
    GameQuitException(const std::string& msg) : GameException(msg) {}
    virtual ~GameQuitException() throw() {}
};

#endif
