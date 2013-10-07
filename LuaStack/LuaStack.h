#pragma once

#include <string>
#include <lua.hpp>

namespace LuaLink
{
	struct LuaStack
	{
		template<typename T> 
		static T getVariable(lua_State* pLua, int varIdx, bool& isOk);

		template<typename T> 
		static T getVariable(lua_State* pLua, int varIdx);

		template<typename T> 
		static void pushVariable(lua_State* pLua, T data);
	
		template<typename... T>
		static void pushStack(lua_State* pLua, T... data);

	private:
		template<typename... T> struct Implementation_getStack;

		template<typename... T>	struct Implementation_pushStack;
		
		//Disable default constructor, destructor, copy constructor & assignment operator
		LuaStack(void);
		~LuaStack(void);
		LuaStack(const LuaStack& src);
		LuaStack& operator=(const LuaStack& src);
	};
}
#include "LuaStack.inl"
