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

#include "LuaStack.hpp"

#include <iterator>

namespace LuaLink
{
    namespace detail {
        namespace LuaFunction {
            //Struct form of wrapper/callbacks, necessary to keep a lookup table of all wrappers/callbacks
            struct Unsafe_LuaFunc{
                Unsafe_LuaFunc(WrapperDoubleArg w1, WrapperSingleArg w2, void* cb) : pWrapper(w1), pWrapperSingle(w2), pFunc(cb){}
                
                WrapperDoubleArg pWrapper; //Used for calls to overloaded member functions
                WrapperSingleArg pWrapperSingle; //Used for calls to non-overloaded member functions
                void* pFunc; //Serves as callback, discards type, wrappers restore type
            };
            
            extern void Register_Impl(Unsafe_LuaFunc&&,const char*);
        }
    }
    
	template<typename _RetType, typename... _ArgTypes>
	// // Add a C++ member function to the appropriate lookup table
	void LuaFunction::Register(_RetType(*pFunc)(_ArgTypes...), const char* name)
    {
        using namespace detail;
        using namespace detail::LuaFunction;
        detail::LuaFunction::Register_Impl(Unsafe_LuaFunc(
                                     FunctionWrapper<_RetType, _ArgTypes...>::execute,
                                     FunctionWrapper<_RetType, _ArgTypes...>::execute,
                                     reinterpret_cast<void*>(pFunc)), name);
	}
}

namespace LuaLink {
    
    #ifdef LUALINK_DEFINE
    namespace detail {
        namespace LuaFunction {
            std::map<const char*, std::vector<Unsafe_LuaFunc>, CStrCmp>& LuaFunctionMap() {
                static std::map<const char*, std::vector<Unsafe_LuaFunc>, CStrCmp> s;
                return s;
            }
            
            std::vector<Unsafe_LuaFunc>& LuaFunctionTable() {
                static std::vector<Unsafe_LuaFunc> s;
                return s;
            }
            
            void Register_Impl(Unsafe_LuaFunc&& func, const char* name) {
                    auto it = LuaFunctionMap().find(name);
                    if( it == LuaFunctionMap().end() )
                        it = LuaFunctionMap().insert(make_pair(name, std::vector<Unsafe_LuaFunc>() ) ).first;
                    it->second.push_back(func);
            }
        }
    }
    
    // // Pushes all registered functions to the Lua environment
    void LuaFunction::Commit(lua_State* pLuaState)
    {
        using namespace detail::LuaFunction;
        
        for(auto& elem : LuaFunctionMap()) {
            //No overloading
            if(elem.second.size() == 1){
                lua_pushlightuserdata(pLuaState, elem.second[0].pFunc);
                lua_pushcclosure(pLuaState, elem.second[0].pWrapperSingle, 1);
                lua_setglobal(pLuaState, elem.first );
                continue;
            }
            
            //Copy function objects to table
            size_t startIdx = LuaFunctionTable().size();
            size_t endIdx = startIdx + elem.second.size();
            
            std::move(elem.second.begin(), elem.second.end(), std::insert_iterator<std::vector<Unsafe_LuaFunc>>(LuaFunctionTable(), LuaFunctionTable().end()));
            
            //Push start and end indices
            lua_pushinteger(pLuaState, (lua_Integer)startIdx);
            lua_pushinteger(pLuaState, (lua_Integer)endIdx);
            
            //Push closure
            lua_pushcclosure(pLuaState, LuaFunctionDispatch, 2);
            lua_setglobal(pLuaState, elem.first);
        }
        LuaFunctionMap().clear();
    }
    
    // // Throws out all references to functions that are left over from previous commits
    void LuaFunction::Release(void)
    {
        using namespace detail::LuaFunction;
        if(!LuaFunctionTable().empty())
            LuaFunctionTable().clear();
    }
    
    // Tries out all overloads until it finds an overload that matches the arguments used in the Lua call
    int LuaFunction::LuaFunctionDispatch(lua_State* L)
    {
        using namespace detail::LuaFunction;
        auto startIdx = static_cast<int>( lua_tointeger( L, lua_upvalueindex(1) ) );
        auto endIdx =	static_cast<int>( lua_tointeger( L, lua_upvalueindex(2) ) );
        
        for(auto i = startIdx; i < endIdx; ++i){
            auto ret = LuaFunctionTable()[i].pWrapper(L, LuaFunctionTable()[i].pFunc, OverloadedErrorHandling);
            if(ret < 0)
                continue;
            
            return ret;
        }
        return luaL_error(L, "Invalid function call");
    }
    
    int LuaFunction::DefaultErrorHandling(lua_State* L, int narg)	{ return narg == 0 ? luaL_error(L, "Bad # of arguments") : luaL_argerror(L, narg,""); }
    int LuaFunction::OverloadedErrorHandling(lua_State* L, int narg){ return -1; }
#endif //LUALINK_DEFINE

	// CALLBACK WRAPPERS

	#define EXECUTE_V2	static int execute(lua_State* pLuaState){return execute(pLuaState, lua_touserdata( pLuaState, lua_upvalueindex(1) ), ::LuaLink::LuaFunction::DefaultErrorHandling);}

    namespace detail {
        //functionwrapper
        
        //ret, n args
        template<typename _RetType, typename... _ArgTypes>
        struct FunctionWrapper
        {
            typedef _RetType(*CbType)(_ArgTypes...);
            
            static int execute(lua_State* pLuaState, void* fn, ArgErrorCbType onArgError)
            {
                bool isOk = lua_gettop(pLuaState) == sizeof...(_ArgTypes);
                if(!isOk)
                    return onArgError(pLuaState, 0);
                
                int err = 0;
                auto tpl = build_tuple_from_lua_stack<_ArgTypes...>::execute(pLuaState, 1, isOk, onArgError, err);
                if(!isOk)
                    return err;
                
                LuaStack::pushVariable<_RetType>( pLuaState, call(reinterpret_cast<CbType>(fn), tpl) );
                return 1;
            }
            
            EXECUTE_V2
        };
        
        //no ret, n args
        template<typename... _ArgTypes>
        struct FunctionWrapper<void, _ArgTypes...>
        {
            typedef void(*CbType)(_ArgTypes...);
            
            static int execute(lua_State* pLuaState, void* fn, ArgErrorCbType onArgError)
            {
                bool isOk = lua_gettop(pLuaState) == sizeof...(_ArgTypes);
                if(!isOk)
                    return onArgError(pLuaState, 0);
                
                int err = 0;
                auto tpl = build_tuple_from_lua_stack<_ArgTypes...>::execute(pLuaState, 1, isOk, onArgError, err);
                if(!isOk)
                    return err;
                
                call(reinterpret_cast<CbType>(fn), tpl);
                return 0;
            }
            
            EXECUTE_V2
        };
        
        //ret, 0 arg
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
            
            EXECUTE_V2
        };
    }

	#undef EXECUTE_V2
}
