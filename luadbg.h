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
