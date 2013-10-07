#include "LuaVariable.h"

using namespace LuaLink;

std::vector<LuaVariable::Unsafe_VariableWrapper> LuaVariable::s_VariablesToCommit = std::vector<LuaVariable::Unsafe_VariableWrapper>();

void LuaVariable::Commit(lua_State* pLuaState, int tableIdx)
{
	for(auto& elem : s_VariablesToCommit)
	{
		if(tableIdx != 0)
			lua_pushstring(pLuaState, elem.Name.c_str());

		lua_newtable(pLuaState);
			
		lua_pushstring(pLuaState, "get");
		lua_pushlightuserdata(pLuaState, elem.Data);
		lua_pushcclosure(pLuaState, elem.Getter, 1);
		lua_settable(pLuaState, -3);
			
		lua_pushstring(pLuaState, "set");
		lua_pushlightuserdata(pLuaState, elem.Data);
		lua_pushcclosure(pLuaState, elem.Setter, 1);
		lua_settable(pLuaState, -3);
			
		if(tableIdx == 0)
			lua_setglobal(pLuaState, elem.Name.c_str());
		else
			lua_settable(pLuaState, tableIdx);
	}

	s_VariablesToCommit.clear();
}
