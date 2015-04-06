// Copyright � 2013 Tom Tondeur
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

#include <string>
#include <lua.hpp>
#include "../LuaStack/LuaStack.h"

namespace LuaLink
{
	template<typename T>
	// // Register a variable of type T, will be automatically registered for a class if done so in the appropriate member function
	void LuaVariable::Register(T& var, const char* varName)
	{
		s_VariablesToCommit.push_back(Unsafe_VariableWrapper(&var, varName));
	}

	//Intermediate format to store variables as they wait to be commited to Lua
	struct LuaVariable::Unsafe_VariableWrapper
	{
		template<typename T>
		Unsafe_VariableWrapper(T* pVar, const std::string& name) : Data(static_cast<void*>(pVar)), 
																	Name(name), 
																	Getter(Implementation<T>::get), 
																	Setter(Implementation<T>::set) {}
		void* Data;
		lua_CFunction Getter;
		lua_CFunction Setter;
		std::string Name;
	};

	template<typename T>
	//Getter for auto-properties
	int LuaVariable::Implementation<T>::get(lua_State* L)
	{
		lua_pushinteger(L, *static_cast<T*>(lua_touserdata(L, lua_upvalueindex(1))));
		return 1;
	}

	template<typename T>
	//Setter for auto-properties
	int LuaVariable::Implementation<T>::set(lua_State* L)
	{
		*static_cast<T*>(lua_touserdata(L, lua_upvalueindex(1))) = LuaStack::getVariable<T>(L, 1);
		return 1;
	}
}