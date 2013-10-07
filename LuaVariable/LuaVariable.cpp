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

#include "LuaVariable.h"

using namespace LuaLink;

std::vector<LuaVariable::Unsafe_VariableWrapper> LuaVariable::s_VariablesToCommit = std::vector<LuaVariable::Unsafe_VariableWrapper>();

void LuaVariable::Commit(lua_State* pLuaState, int tableIdx)
{
	//For each variable
	for(auto& elem : s_VariablesToCommit)
	{
		//Push string to the stack when registering for a table, so we can call settable later
		if(tableIdx != 0)
			lua_pushstring(pLuaState, elem.Name.c_str());

		//Create new table to hold 'get' and 'set' methods
		lua_newtable(pLuaState);
			
		//Push getter
		lua_pushstring(pLuaState, "get");
		lua_pushlightuserdata(pLuaState, elem.Data);
		lua_pushcclosure(pLuaState, elem.Getter, 1);
		lua_settable(pLuaState, -3);
			
		//Push setter
		lua_pushstring(pLuaState, "set");
		lua_pushlightuserdata(pLuaState, elem.Data);
		lua_pushcclosure(pLuaState, elem.Setter, 1);
		lua_settable(pLuaState, -3);
			
		//Set as global or as field in a table
		if(tableIdx == 0)
			lua_setglobal(pLuaState, elem.Name.c_str());
		else
			lua_settable(pLuaState, tableIdx);
	}

	//Flush the cache of variables to commit so this can be re-used
	s_VariablesToCommit.clear();
}
