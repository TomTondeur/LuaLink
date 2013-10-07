#pragma once

#include <lua.hpp>

namespace LuaLink
{
	template<typename T>
	void LuaVariable::Register(T& var, const char* varName)
	{
		s_VariablesToCommit.push_back(Unsafe_VariableWrapper(&var, varName));
	}

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
	int LuaVariable::Implementation<T>::get(lua_State* L)
	{
		lua_pushinteger(L, *static_cast<T*>(lua_touserdata(L, lua_upvalueindex(1))));
		return 1;
	}

	template<typename T>
	int LuaVariable::Implementation<T>::set(lua_State* L)
	{
		*static_cast<T*>(lua_touserdata(L, lua_upvalueindex(1))) = LuaStack::getVariable<T>(L, 1);
		return 1;
	}
}