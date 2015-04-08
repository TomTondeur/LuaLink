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

namespace LuaLink
{
	template<typename ClassT>
	std::map<std::string, std::vector<LuaFunction::Unsafe_LuaFunc> > LuaStaticMethod<ClassT>::s_LuaFunctionMap;

	template<typename ClassT>
	int(*LuaStaticMethod<ClassT>::s_OverloadedConstructorWrapper)(lua_State*, detail::WrapperDoubleArg, void*, detail::ArgErrorCbType onArgError) = nullptr;

	template<typename ClassT>
	template<typename FunctionType>
	void LuaStaticMethod<ClassT>::Register(FunctionType pFunc, const std::string& name){
		Register_Impl(pFunc, name);
	}

	template<typename ClassT>
	template<typename _RetType, typename... _ArgTypes>
	void LuaStaticMethod<ClassT>::Register_Impl(_RetType(*pFunc)(_ArgTypes...), const std::string& name)
	{
		//Get iterator that points to the lookup table associated with 'name'
		auto it = s_LuaFunctionMap.find(name);
		if( it == s_LuaFunctionMap.end() )
			it = s_LuaFunctionMap.insert(make_pair(name, std::vector<LuaFunction::Unsafe_LuaFunc>() ) ).first;
	
		//Add wrapper to lookup table
		it->second.push_back(LuaFunction::Unsafe_LuaFunc(
                                                         detail::FunctionWrapper<_RetType, _ArgTypes...>::execute,
                                                         detail::FunctionWrapper<_RetType, _ArgTypes...>::execute,
                                                         reinterpret_cast<void*>(pFunc)));
	}
	
	template<typename ClassT>
	void LuaStaticMethod<ClassT>::Commit(lua_State* pLuaState, int metatable)
	{
		for(auto& elem : s_LuaFunctionMap){
			//No overloading
			if(elem.second.size() == 1){
				lua_pushstring(pLuaState, elem.first.c_str() ); //Push function name

				lua_pushlightuserdata(pLuaState, elem.second[0].pFunc); //Push callback
				lua_pushcclosure(pLuaState, elem.second[0].pWrapperSingle, 1); //Push wrapper
				
				lua_settable(pLuaState, metatable); //Add entry to the Lua table
				continue;
			}
		
			//This function needs to be overloaded =>

			//Move functions to lookup table
			size_t startIdx = LuaFunction::s_LuaFunctionTable.size();
			size_t endIdx = startIdx + elem.second.size();

			std::move(elem.second.begin(), elem.second.end(), std::insert_iterator<std::vector<LuaFunction::Unsafe_LuaFunc>>(LuaFunction::s_LuaFunctionTable, LuaFunction::s_LuaFunctionTable.end()));

			lua_pushstring(pLuaState, elem.first.c_str()); //Push function name
		
			//Push start and end indices of functions in lookup table (endIdx is one past last function, like .end() iterators)
			lua_pushinteger(pLuaState, startIdx);
			lua_pushinteger(pLuaState, endIdx);

			lua_pushcclosure(pLuaState, LuaFunction::LuaFunctionDispatch, 2); //Push closure

			lua_settable(pLuaState, metatable); //Set table
		}
        s_LuaFunctionMap.clear();
	}

	template<typename ClassT>
	void LuaStaticMethod<ClassT>::CommitConstructors(lua_State* pLuaState, int metatable, lua_CFunction ctorWrapper, int(*overloadedCtorWrapper)(lua_State*, detail::WrapperDoubleArg, void*, detail::ArgErrorCbType onArgError))
	{
		s_OverloadedConstructorWrapper = overloadedCtorWrapper;

		auto it = s_LuaFunctionMap.find("new");
		if(it == s_LuaFunctionMap.end())
			return;

		//No overloading
		if(it->second.size() == 1){
			lua_pushstring(pLuaState, "new" ); //Push function name
		
			lua_pushlightuserdata(pLuaState, reinterpret_cast<void*>(it->second[0].pWrapper)); //Push wrapper callback
			lua_pushlightuserdata(pLuaState, it->second[0].pFunc); //Push callback
			lua_pushcclosure(pLuaState, ctorWrapper, 2); //Push ctor wrapper
				
			lua_settable(pLuaState, metatable); //Add entry to the Lua table

			s_LuaFunctionMap.erase(it);
			return;
		}
		
		//This function needs to be overloaded =>

		//Move functions to lookup table
		size_t startIdx = LuaFunction::s_LuaFunctionTable.size();
		size_t endIdx = startIdx + it->second.size();

		std::move(it->second.begin(), it->second.end(), std::insert_iterator<std::vector<LuaFunction::Unsafe_LuaFunc>>(LuaFunction::s_LuaFunctionTable, LuaFunction::s_LuaFunctionTable.end()));

		lua_pushstring(pLuaState, "new"); //Push function name
		
		//Push start and end indices of functions in lookup table (endIdx is one past last function, like .end() iterators)
		lua_pushinteger(pLuaState, startIdx);
		lua_pushinteger(pLuaState, endIdx);

		lua_pushcclosure(pLuaState, OverloadedCTorDispatch, 2); //Push closure

		lua_settable(pLuaState, metatable); //Set table
	
		s_LuaFunctionMap.erase(it);
	}

	template<typename ClassT>
	int LuaStaticMethod<ClassT>::OverloadedCTorDispatch(lua_State* L)
	{
		//Get locations of valid constructors in our lookup table
		auto startIdx = lua_tointeger( L, lua_upvalueindex(1) );
		auto endIdx =	lua_tointeger( L, lua_upvalueindex(2) );

		//Try constructors until we find one that fits (in case of failure, they will return before allocating any memory)
		for(auto i = startIdx; i < endIdx; ++i){
			int ret = s_OverloadedConstructorWrapper(L, LuaFunction::s_LuaFunctionTable[i].pWrapper, LuaFunction::s_LuaFunctionTable[i].pFunc, LuaFunction::OverloadedErrorHandling);
		
			if(ret < 0)
				continue;

			return ret;
		}
		//No valid overload has been found
		return luaL_error(L, "Unable to match a constructor with the provided input signature.");
	}
}