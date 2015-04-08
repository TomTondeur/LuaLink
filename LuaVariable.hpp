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

#include <vector>
#include <lua.hpp>

namespace LuaLink
{
	template <typename T> class LuaClass;

	class LuaVariable
	{
    public:
		template<typename T>
		// // Register a variable of type T, will be automatically registered for a class if done so in the appropriate member function
		static void Register(T& var, const char* varName);

	private:
		//LuaScript and LuaClass need to trigger the actual commits to the lua_State
		friend class LuaScript;
		template<typename T> friend class LuaClass;

		//Temporary format for our variables
		struct Unsafe_VariableWrapper;

		//Temporary container for variables that haven't been commited yet
		static std::vector<Unsafe_VariableWrapper> s_VariablesToCommit;

		// // Commits all registered variables to the lua_State, pass 0 to register as global, otherwise the index on the stack of the table to register variables for
		static void Commit(lua_State* pLuaState, int tableIdx = 0);

		template<typename T>
		//Callbacks to get and set variables in Lua, comparable to auto-properties in C#
		struct Implementation
		{
			static int get(lua_State* L);
			static int set(lua_State* L);
		};
		
		//Disable default constructor, destructor, copy constructor & assignment operator
		LuaVariable(void) = delete;
		~LuaVariable(void) = delete;
		LuaVariable(const LuaVariable& src) = delete;
		LuaVariable& operator=(const LuaVariable& src) = delete;
	};
}

#include "LuaVariable.inl"
