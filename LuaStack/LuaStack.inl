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

namespace LuaLink
{
	//Partial specialization for LuaStack::Implementation_pushStack

	template<typename H, typename... T>
	struct LuaStack::Implementation_pushStack<H, T...>{
		static void pushStack(lua_State* pLua, H data, T... tailData) 
		{ 
			pushVariable<H>(pLua, data);
            pushStack(pLua, tailData...);
		}
	};

	template<typename T>
	struct LuaStack::Implementation_pushStack<T>{
		static void pushStack(lua_State* pLua, T data) 
		{ 
			pushVariable<T>(pLua, data);
		}
	};

	template<>
	struct LuaStack::Implementation_pushStack<>{
		static void pushStack(lua_State* pLua) { }
	};

	template<typename... T>
	void LuaStack::pushStack(lua_State* pLua, T... data)
	{
		Implementation_pushStack<T...>::pushStack(pLua, data...);
	}
}
