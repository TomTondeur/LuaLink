#pragma once

namespace LuaLink
{
	//pushStack

	template<typename H, typename... T>
	struct LuaStack::Implementation_pushStack<H, T...>{
		static void pushStack(lua_State* pLua, H data, T... tailData) 
		{ 
			pushVariable<H>(pLua, data); pushStack<T...>(pLua, tailData...);
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
