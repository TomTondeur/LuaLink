// Copyright © 2013 Tom Tondeur
// 
// This file is part of LuaLink.
// 
// LuaLink is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// LuaLink is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with LuaLink.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include <lua.hpp>
#include <iostream>
#include <stdexcept>

struct LuaLoadException : public std::runtime_error
{
    explicit LuaLoadException(const std::string& msg):std::runtime_error(msg){}
};

struct LuaCallException : public std::runtime_error
{
	explicit LuaCallException(const std::string& msg):std::runtime_error(msg){}
};
