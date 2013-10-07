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

#include "LuaFunction.h"
using namespace LuaLink;

// // Pushes all registered functions to the Lua environment
void LuaFunction::Commit(lua_State* pLuaState)
{
	for(auto& elem : s_LuaFunctionMap){
		//No overloading
		if(elem.second.size() == 1){
			lua_pushlightuserdata(pLuaState, elem.second[0].pFunc);
			lua_pushcclosure(pLuaState, elem.second[0].pWrapperSingle, 1);
			lua_setglobal(pLuaState, elem.first.c_str() );
			continue;
		}

		//Copy function objects to table
		unsigned int startIdx = s_LuaFunctionTable.size();
		unsigned int endIdx = startIdx + elem.second.size();

		std::move(elem.second.begin(), elem.second.end(), std::insert_iterator<std::vector<Unsafe_LuaFunc>>(s_LuaFunctionTable, s_LuaFunctionTable.end()));

		//Push start and end indices
		lua_pushunsigned(pLuaState, startIdx);
		lua_pushunsigned(pLuaState, endIdx);

		//Push closure
		lua_pushcclosure(pLuaState, LuaFunctionDispatch, 2);
		lua_setglobal(pLuaState, elem.first.c_str());
	}
	s_LuaFunctionMap.clear();
	s_LuaFunctionMap.swap(std::map<std::string, std::vector<LuaFunction::Unsafe_LuaFunc>>());
}

// // Throws out all references to functions that are left over from previous commits
void LuaFunction::Release(void)
{
	if(!s_LuaFunctionTable.empty())
		s_LuaFunctionTable.clear();
}
	
// Tries out all overloads until it finds an overload that matches the arguments used in the Lua call
int LuaFunction::LuaFunctionDispatch(lua_State* L)
{
	unsigned int startIdx = lua_tounsigned( L, lua_upvalueindex(1) );
	unsigned int endIdx =	lua_tounsigned( L, lua_upvalueindex(2) );

	for(unsigned int i = startIdx; i < endIdx; ++i){
		int ret = s_LuaFunctionTable[i].pWrapper(L, s_LuaFunctionTable[i].pFunc, OverloadedErrorHandling);
		if(ret < 0)
			continue;
		
		return ret;
	}
	return luaL_error(L, "Invalid function call");
}

int LuaFunction::DefaultErrorHandling(lua_State* L, int narg)	{ return narg == 0 ? luaL_error(L, "Bad # of arguments") : luaL_argerror(L, narg,""); }
int LuaFunction::OverloadedErrorHandling(lua_State* L, int narg){ return -1; }

std::map<std::string, std::vector<LuaFunction::Unsafe_LuaFunc> > LuaFunction::s_LuaFunctionMap = std::map<std::string, std::vector<LuaFunction::Unsafe_LuaFunc> >();
std::vector<LuaFunction::Unsafe_LuaFunc> LuaFunction::s_LuaFunctionTable = std::vector<LuaFunction::Unsafe_LuaFunc>();
