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

#include "../LuaStack/LuaStack.h"
#include <vector>
#include <map>

namespace LuaLink
{
	template<typename ClassT>
	template<typename MemberFunctionT>
	// // Add a C++ member function to the Lua environment
	void LuaMethod<ClassT>::Register(MemberFunctionT pFunc, const std::string& name){
		Register_Impl(pFunc, name);
	}
	
	template<typename ClassT>
	//Struct form of wrapper/callbacks, necessary to keep a lookup table of all wrappers/callbacks
	struct LuaMethod<ClassT>::Unsafe_MethodWrapper{
		Unsafe_MethodWrapper(WrapperDoubleArg w1, WrapperSingleArg w2, Unsafe_MethodType cb) : pWrapper(w1), pWrapperSingle(w2), pFunc(cb){}

		WrapperDoubleArg pWrapper; //Used for calls to overloaded member functions
		WrapperSingleArg pWrapperSingle; //Used for calls to non-overloaded member functions
		Unsafe_MethodType pFunc; //Serves as callback, discards return & argument types, wrappers restore return & argument types
	};

	template<typename ClassT>
	template<typename _RetType, typename... _ArgTypes>
	// // Add a C++ member function to the appropriate lookup table
	void LuaMethod<ClassT>::Register_Impl(_RetType(ClassT::*pFunc)(_ArgTypes...), const std::string& name)
	{
		//Get iterator that points to the lookup table associated with 'name'
		auto it = s_LuaFunctionMap.find(name);
		if( it == s_LuaFunctionMap.end() )
			it = s_LuaFunctionMap.insert(make_pair(name, std::vector<Unsafe_MethodWrapper>() ) ).first;

		//Add wrapper to lookup table
        auto test = Unsafe_MethodWrapper(detail::MethodWrapper<ClassT, _RetType, _ArgTypes...>::execute,
                                         detail::MethodWrapper<ClassT, _RetType, _ArgTypes...>::execute,
                                         reinterpret_cast<Unsafe_MethodType>(pFunc));
        it->second.push_back(test);
	}

	template<typename ClassT>
	// // Pushes all registered member functions to the Lua environment
	void LuaMethod<ClassT>::Commit(lua_State* pLuaState, int tablePosOnStack)
	{
		if(!s_LuaFunctionTable.empty())
			s_LuaFunctionTable.clear();

		for(auto& elem : s_LuaFunctionMap)
		{
			//No overloading
			if(elem.second.size() == 1){
				s_LuaFunctionTable.push_back(elem.second[0]); //Add function to the lookup table

				lua_pushstring(pLuaState, elem.first.c_str() ); //Push function name

				//Push index of function in lookup table & wrapper as closure
				lua_pushinteger(pLuaState, s_LuaFunctionTable.size() - 1);
				lua_pushcclosure(pLuaState, elem.second[0].pWrapperSingle, 1);
				
				lua_settable(pLuaState, tablePosOnStack); //Add entry to the Lua table
				continue;
			}

			//This function needs to be overloaded =>

			//Move functions to lookup table
			size_t startIdx = s_LuaFunctionTable.size();
			size_t endIdx = startIdx + elem.second.size();

			std::move(elem.second.begin(), elem.second.end(), std::insert_iterator<std::vector<Unsafe_MethodWrapper>>(s_LuaFunctionTable, s_LuaFunctionTable.end()));
		
			lua_pushstring(pLuaState, elem.first.c_str()); //Push function name

			//Push start and end indices of functions in lookup table (endIdx is one past last function, like .end() iterators)
			lua_pushinteger(pLuaState, startIdx);
			lua_pushinteger(pLuaState, endIdx);

			lua_pushcclosure(pLuaState, OverloadDispatch, 2); //Push closure

			lua_settable(pLuaState, tablePosOnStack); //Set table
		}
        s_LuaFunctionMap.clear();
	}

	template<typename ClassT>
	void LuaMethod<ClassT>::PushThisPointer(lua_State* L)
	{
		//Make sure the object was pushed to the stack
		if(lua_istable(L, 1) == 0)
			luaL_error(L, "Calling a nonstatic member function requires a reference to an object");

		//Push ppObj to stack
		lua_pushstring(L, "core_");
		lua_rawget(L, 1); 
	
		if(lua_isuserdata(L, -1) == 0)
			luaL_error(L, "Calling a nonstatic member function requires a reference to an object");
	
		lua_remove(L, 1); //Remove 'self table' from stack
	}
	
	template<typename ClassT>
	// Tries out all overloads until it finds an overload that matches the arguments used in the Lua call
	int LuaMethod<ClassT>::OverloadDispatch(lua_State* L)
	{
		//Retrieve range in lookup table where overloads are located
		auto startIdx = lua_tointeger( L, lua_upvalueindex(1) );
		auto endIdx =	lua_tointeger( L, lua_upvalueindex(2) );

		PushThisPointer(L);

		//Try all functions
		for(auto i = startIdx; i < endIdx; ++i){
			int ret = s_LuaFunctionTable[i].pWrapper(L, s_LuaFunctionTable[i].pFunc, LuaFunction::OverloadedErrorHandling);
		
			if(ret < 0)
				continue;
		
			return ret;
		}

		//No correct overload found
		return luaL_error(L, "Invalid member function call - no overload found that takes these parameters.");
	}

	template<typename ClassT>
	// // Common code in all MethodWrappers, returns pointer to pointer to object to call member function on
	ClassT** LuaMethod<ClassT>::GetObjectAndVerifyStackSize(lua_State* L, int nrOfArgs)
	{
		int argc = lua_gettop(L);
		if(argc != nrOfArgs + 1) //stack should contain 'this pointer' + args
			return nullptr;
	
		auto ppObj = static_cast<ClassT**>(lua_touserdata(L, -1) ); //Retrieve internal object
		lua_pop(L, 1); //Remove internal object from stack
		return ppObj; //Return internal object
	}
	
	// CALLBACK WRAPPERS

	#define GET_THIS(ARGC) auto ppObj = LuaMethod<ClassT>::GetObjectAndVerifyStackSize(pLuaState, ARGC); \
        if(!ppObj) \
            return onArgError(pLuaState, 0); \
        bool isOk = true; \
        (void)isOk;
    
	#define GET_ARG(ARGN) auto arg##ARGN = LuaStack::getVariable<_Arg##ARGN##Type>(pLuaState, ARGN, isOk); \
        if(!isOk) \
            return onArgError(pLuaState, ARGN);
    
	#define DO_LUACALLBACK(CBTYPE,...) ((*ppObj)->*(reinterpret_cast<CBTYPE>(fn)))( __VA_ARGS__ )

	#define EXECUTE_V2 static int execute(lua_State* L){ \
		LuaMethod<ClassT>::PushThisPointer(L);\
		return execute(L, LuaMethod<ClassT>::s_LuaFunctionTable[static_cast<unsigned int>(lua_tointeger( L, lua_upvalueindex(1) ) )].pFunc, LuaFunction::DefaultErrorHandling);}

    namespace detail {
        //no ret, 1 arg
        template<typename ClassT, typename _Arg1Type>
        struct MethodWrapper<ClassT, void, _Arg1Type>
        {
            static int execute(lua_State* pLuaState, typename LuaMethod<ClassT>::Unsafe_MethodType fn, ArgErrorCbType onArgError)
            {
                GET_THIS(1)
                GET_ARG(1)
                DO_LUACALLBACK(void(ClassT::*)(_Arg1Type),arg1);
                return 0;
            }
            
            EXECUTE_V2
        };
        
        //no ret, 2 arg
        template<typename ClassT, typename _Arg1Type, typename _Arg2Type>
        struct MethodWrapper<ClassT, void, _Arg1Type, _Arg2Type>
        {
            static int execute(lua_State* pLuaState, typename LuaMethod<ClassT>::Unsafe_MethodType fn, ArgErrorCbType onArgError)
            {
                GET_THIS(2)
                GET_ARG(1)
                GET_ARG(2)
                DO_LUACALLBACK(void(ClassT::*)(_Arg1Type, _Arg2Type), arg1, arg2);
                return 0;
            }
            
            EXECUTE_V2
        };
        
        //	 ret, 0 arg
        template<typename ClassT, typename _RetType>
        struct MethodWrapper<ClassT, _RetType>
        {
            static int execute(lua_State* pLuaState, typename LuaMethod<ClassT>::Unsafe_MethodType fn, ArgErrorCbType onArgError)
            {
                GET_THIS(0)
                LuaStack::pushVariable(	pLuaState, DO_LUACALLBACK( _RetType(ClassT::*)(void) ) );
                return 1;
            }
            
            EXECUTE_V2
        };
        
        //	 ret, 1 arg
        template<typename ClassT, typename _RetType, typename _Arg1Type>
        struct MethodWrapper<ClassT, _RetType, _Arg1Type>
        {
            static int execute(lua_State* pLuaState, typename LuaMethod<ClassT>::Unsafe_MethodType fn, ArgErrorCbType onArgError)
            {
                GET_THIS(1)
                GET_ARG(1)
                LuaStack::pushVariable(	pLuaState, DO_LUACALLBACK(_RetType(ClassT::*)(_Arg1Type), arg1) );
                return 1;
            }
            
            EXECUTE_V2
        };
        
        //	 ret, 2 arg
        template<typename ClassT, typename _RetType, typename _Arg1Type, typename _Arg2Type>
        struct MethodWrapper<ClassT, _RetType, _Arg1Type, _Arg2Type>
        {
            static int execute(lua_State* pLuaState, typename LuaMethod<ClassT>::Unsafe_MethodType fn, ArgErrorCbType onArgError)
            {
                GET_THIS(2)
                GET_ARG(1)
                GET_ARG(2)
                LuaStack::pushVariable(	pLuaState, DO_LUACALLBACK(_RetType(ClassT::*)(_Arg1Type, _Arg2Type), arg1, arg2) );
                return 1;
            }
            
            EXECUTE_V2
        };
    }
    
#undef GET_THIS
#undef GET_ARG
#undef DO_LUACALLBACK
#undef EXECUTE_V2
    
    template<typename ClassT>
    std::vector<typename LuaMethod<ClassT>::Unsafe_MethodWrapper> LuaMethod<ClassT>::s_LuaFunctionTable;
    
    template<typename ClassT>
    std::map<std::string, std::vector<typename LuaMethod<ClassT>::Unsafe_MethodWrapper> > LuaMethod<ClassT>::s_LuaFunctionMap;
    
}
