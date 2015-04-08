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

#include "LuaStack.h"

namespace LuaLink
{
	template<typename FunctionType>
	// // Add a C++ global function to the Lua environment
	void LuaFunction::Register(FunctionType pFunc, const std::string& name)
	{
		Register_Impl(pFunc, name);
	}
		
	template<typename _RetType, typename... _ArgTypes>
	// // Add a C++ member function to the appropriate lookup table
	void LuaFunction::Register_Impl(_RetType(*pFunc)(_ArgTypes...), const std::string& name)
	{
		auto it = s_LuaFunctionMap.find(name);
		if( it == s_LuaFunctionMap.end() )
			it = s_LuaFunctionMap.insert(make_pair(name, std::vector<Unsafe_LuaFunc>() ) ).first;
        it->second.push_back(Unsafe_LuaFunc(
                                            detail::FunctionWrapper<_RetType, _ArgTypes...>::execute,
                                            detail::FunctionWrapper<_RetType, _ArgTypes...>::execute,
                                            reinterpret_cast<void*>(pFunc)));
	}

	//Struct form of wrapper/callbacks, necessary to keep a lookup table of all wrappers/callbacks
	struct LuaFunction::Unsafe_LuaFunc{
        Unsafe_LuaFunc(detail::WrapperDoubleArg w1, detail::WrapperSingleArg w2, void* cb) : pWrapper(w1), pWrapperSingle(w2), pFunc(cb){}

        detail::WrapperDoubleArg pWrapper; //Used for calls to overloaded member functions
		detail::WrapperSingleArg pWrapperSingle; //Used for calls to non-overloaded member functions
		void* pFunc; //Serves as callback, discards type, wrappers restore type
	};
    
#ifdef LUALINK_DEFINE
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
            size_t startIdx = s_LuaFunctionTable.size();
            size_t endIdx = startIdx + elem.second.size();
            
            std::move(elem.second.begin(), elem.second.end(), std::insert_iterator<std::vector<Unsafe_LuaFunc>>(s_LuaFunctionTable, s_LuaFunctionTable.end()));
            
            //Push start and end indices
            lua_pushinteger(pLuaState, (lua_Integer)startIdx);
            lua_pushinteger(pLuaState, (lua_Integer)endIdx);
            
            //Push closure
            lua_pushcclosure(pLuaState, LuaFunctionDispatch, 2);
            lua_setglobal(pLuaState, elem.first.c_str());
        }
        s_LuaFunctionMap.clear();
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
        auto startIdx = static_cast<lua_Unsigned>( lua_tointeger( L, lua_upvalueindex(1) ) );
        auto endIdx =	static_cast<lua_Unsigned>( lua_tointeger( L, lua_upvalueindex(2) ) );
        
        for(auto i = startIdx; i < endIdx; ++i){
            int ret = s_LuaFunctionTable[i].pWrapper(L, s_LuaFunctionTable[i].pFunc, OverloadedErrorHandling);
            if(ret < 0)
                continue;
            
            return ret;
        }
        return luaL_error(L, "Invalid function call");
    }
    
    int LuaFunction::DefaultErrorHandling(lua_State* L, int narg)	{ return narg == 0 ? luaL_error(L, "Bad # of arguments") : luaL_argerror(L, narg,""); }
    int LuaFunction::OverloadedErrorHandling(lua_State* L, int narg){ return -1; }
    
    std::map<std::string, std::vector<LuaFunction::Unsafe_LuaFunc> > LuaFunction::s_LuaFunctionMap;
    std::vector<LuaFunction::Unsafe_LuaFunc> LuaFunction::s_LuaFunctionTable;
#endif //LUALINK_DEFINE

	// CALLBACK WRAPPERS

	#define CHECK_ARGC(ARGC)	bool isOk = lua_gettop(pLuaState) == ARGC; if(!isOk) return onArgError(pLuaState, 0);
	#define GET_ARG(ARGN)		auto arg##ARGN = LuaStack::getVariable<_Arg##ARGN##Type>(pLuaState, ARGN, isOk); if(!isOk) return onArgError(pLuaState, ARGN);

	#define EXECUTE_V2	static int execute(lua_State* pLuaState){return execute(pLuaState, lua_touserdata( pLuaState, lua_upvalueindex(1) ), LuaFunction::DefaultErrorHandling);}

    namespace detail {
        //no ret, 0 arg
        template<>struct FunctionWrapper<void>
        {
            typedef void(*CbType)(void);
            
            static int execute(lua_State* pLuaState, void* fn, ArgErrorCbType onArgError)
            {
                if(lua_gettop(pLuaState) != 0) //argc
                    return onArgError(pLuaState, 0);
                
                reinterpret_cast<CbType>(fn)();
                return 0;
            }
            
            static int execute(lua_State* pLuaState){return execute(pLuaState, lua_touserdata( pLuaState, lua_upvalueindex(1) ), LuaFunction::DefaultErrorHandling);}
        };
        
        //no ret, 1 arg
        template<typename _Arg1Type>
        struct FunctionWrapper<void, _Arg1Type>
        {
            typedef void(*CbType)(_Arg1Type);
            
            static int execute(lua_State* pLuaState, void* fn, ArgErrorCbType onArgError)
            {
                CHECK_ARGC(1)
                GET_ARG(1)
                reinterpret_cast<CbType>(fn)(arg1);
                return 0;
            }
            
            EXECUTE_V2
        };
        
        //no ret, 2 arg
        template<typename _Arg1Type, typename _Arg2Type>
        struct FunctionWrapper<void, _Arg1Type, _Arg2Type>
        {
            typedef void(*CbType)(_Arg1Type, _Arg2Type);
            
            static int execute(lua_State* pLuaState, void* fn, ArgErrorCbType onArgError)
            {
                CHECK_ARGC(2)
                GET_ARG(1)
                GET_ARG(2)
                reinterpret_cast<CbType>(fn)(arg1, arg2);
                return 0;
            }
            
            EXECUTE_V2
        };
        
        //	 ret, 0 arg
        template<typename _RetType>
        struct FunctionWrapper<_RetType>
        {
            typedef _RetType(*CbType)(void);
            
            static int execute(lua_State* pLuaState, void* fn, ArgErrorCbType onArgError)
            {
                if(lua_gettop(pLuaState) != 0) //argc
                    return onArgError(pLuaState, 0);
                
                LuaStack::pushVariable<_RetType>( pLuaState, reinterpret_cast<CbType>(fn)() );
                return 1;
            }
            
            EXECUTE_V2
        };
        
        //	 ret, 1 arg
        template<typename _RetType, typename _Arg1Type>
        struct FunctionWrapper<_RetType, _Arg1Type>
        {
            typedef _RetType(*CbType)(_Arg1Type);
            
            static int execute(lua_State* pLuaState, void* fn, ArgErrorCbType onArgError)
            {
                CHECK_ARGC(1)
                GET_ARG(1)			
                LuaStack::pushVariable<_RetType>( pLuaState, reinterpret_cast<CbType>(fn)(arg1) );
                return 1;
            }
            
            EXECUTE_V2
        };
        
        //	 ret, 2 arg
        template<typename _RetType, typename _Arg1Type, typename _Arg2Type>
        struct FunctionWrapper<_RetType, _Arg1Type, _Arg2Type>
        {
            typedef _RetType(*CbType)(_Arg1Type, _Arg2Type);
            
            static int execute(lua_State* pLuaState, void* fn, ArgErrorCbType onArgError)
            {
                CHECK_ARGC(2)
                GET_ARG(1)
                GET_ARG(2)
                LuaStack::pushVariable<_RetType>( pLuaState, reinterpret_cast<CbType>(fn)(arg1, arg2) );
                return 1;
            }
            
            EXECUTE_V2
        };
    }

	#undef CHECK_ARGC
	#undef GET_ARG

	#undef EXECUTE_V2
}
