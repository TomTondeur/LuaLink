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

namespace LuaLink
{
	struct LuaStack
	{
		template<typename T> 
		// // Returns the variable in the Lua stack at varIdx, isOk will be false if we failed to retrieve a value (of the right type)
		static T getVariable(lua_State* pLua, int varIdx, bool& isOk);

		template<typename T> 
		// // Returns the variable in the Lua stack at varIdx, prefer to use the safe overload
		static T getVariable(lua_State* pLua, int varIdx);

		template<typename T> 
		// // Pushes a variable to the Lua stack
		static void pushVariable(lua_State* pLua, T data);
	
		template<typename... T>
		// // Pushes several variables tot he Lua stack simultaneously
		static void pushStack(lua_State* pLua, T... data);

	private:
		//Contains the implementation of pushStack, is partially specialized in LuaStack.inl
		template<typename... T>	struct Implementation_pushStack;
		
		//Disable default constructor, destructor, copy constructor & assignment operator
		LuaStack(void);
		~LuaStack(void);
		LuaStack(const LuaStack& src);
		LuaStack& operator=(const LuaStack& src);
	};
}
#include "LuaStack.inl"
