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

#include <sstream>

#include "../luadbg.h"

#include "../LuaStack/LuaStack.h"

namespace LuaLink
{
	//Utilities for templates
    namespace TemplateUtil{
        namespace detail {
            template <typename... T>
            struct Num_Args;
            
            template<>
            struct Num_Args <>
            {
                static size_t calculate() {
                    return 0;
                }
            };
            
            template <typename H, typename... T>
            struct Num_Args <H, T...>
            {
                static size_t calculate() {
                    return 1 + Num_Args<T...>::calculate();
                }
            };
        }
        
        template <typename... T>
        //Count number of template parameters
        static size_t GetNrOfArguments ()
        {
            return detail::Num_Args<T...>::calculate();
        }
	};

	#define LUA_STATE s_pLuaState.get()
	
	//Call implementations
	template<typename _RetType> //1 return value
	struct LuaScript::Call
	{
		template<typename... _ArgTypes>
		static _RetType LuaFunction(const std::string& functionName, _ArgTypes... arguments)
		{
			//Look for global function with the provided name
			lua_getglobal( LUA_STATE, functionName.c_str() );
			if( lua_type(LUA_STATE, lua_gettop(LUA_STATE)) == LUA_TNIL ){
				lua_settop (LUA_STATE, 0);
				throw LuaCallException( ("Global not found: " + std::string(functionName) ).c_str() );
			}

			//Push arguments onto the Lua stack
			LuaStack::pushStack<_ArgTypes...>(LUA_STATE, arguments...);
		
			//Perform function call
			if (lua_pcall(LUA_STATE, static_cast<int>(TemplateUtil::GetNrOfArguments<_ArgTypes...>()), 1, 0) != 0)
					throw LuaCallException(lua_tostring(LUA_STATE, -1));
		
			//Check return value
			bool isOk = true;
			auto ret = LuaStack::getVariable<_RetType>(LUA_STATE, -1, isOk);
			if(!isOk){
				std::stringstream strstr;
				strstr << "Error: Expected return type " << typeid(_RetType).name() << " does not match the value returned by " << functionName;
				throw LuaCallException(strstr.str().c_str());
			}

			return ret;
		}
	
		template<typename... _ArgTypes>
		static _RetType LuaStaticMethod(const std::string& tableName, const std::string& functionName, _ArgTypes... arguments)
		{
			//Look for global table with the provided name
			lua_getglobal( LUA_STATE, tableName.c_str() );
			if( lua_type(LUA_STATE, lua_gettop(LUA_STATE)) == LUA_TNIL ){
				lua_settop (LUA_STATE, 0);
				throw LuaCallException( ("Global not found: " + std::string(tableName) ).c_str() );
			}

			//Look for function in that table
			lua_getfield(LUA_STATE, -1, functionName.c_str() );
			if(!lua_isfunction(LUA_STATE, -1)){
				lua_settop (LUA_STATE, 0);
				throw LuaCallException( (std::string(functionName) + " is not a function in " + tableName).c_str() );
			}

			//Push arguments onto the Lua stack
			LuaStack::pushStack<_ArgTypes...>(LUA_STATE, arguments...);
		
			//Perform function call
			if (lua_pcall(LUA_STATE, TemplateUtil::GetNrOfArguments<_ArgTypes...>(), 1, 0) != 0) 
					throw LuaCallException(lua_tostring(LUA_STATE, -1));

			//Check return value
			bool isOk = true;
			auto ret = LuaStack::getVariable<_RetType>(LUA_STATE, -1, isOk);
			if(!isOk){
				std::stringstream strstr;
				strstr << "Error: Expected return type " << typeid(_RetType).name() << " does not match the value returned by " << tableName << "::" << functionName;
				throw LuaCallException(strstr.str().c_str());
			}

			return ret;
		}
	};

	template<> //No return value
	struct LuaScript::Call<void>
	{
		template<typename... _ArgTypes>
		static void LuaFunction(const std::string& functionName, _ArgTypes... arguments)
		{
			//Look for global function with the provided name
			lua_getglobal( LUA_STATE, functionName.c_str() );
			if( lua_type(LUA_STATE, lua_gettop(LUA_STATE)) == LUA_TNIL ){
				lua_settop (LUA_STATE, 0);
				throw LuaCallException( ("Global not found: " + std::string(functionName) ).c_str() );
			}
		
			//Push arguments onto the Lua stack
			LuaStack::pushStack<_ArgTypes...>(LUA_STATE, arguments...);
		
			//Perform function call
			if (lua_pcall(LUA_STATE, TemplateUtil::GetNrOfArguments<_ArgTypes...>(), 0, 0) != 0) 
					throw LuaCallException(lua_tostring(LUA_STATE, -1));
		}
	
		template<typename... _ArgTypes>
		static void LuaStaticMethod(const std::string& tableName, const std::string& functionName, _ArgTypes... arguments)
		{
			//Look for global table with the provided name
			lua_getglobal( LUA_STATE, tableName.c_str() );
			if( lua_type(LUA_STATE, lua_gettop(LUA_STATE)) == LUA_TNIL ){
				lua_settop (LUA_STATE, 0);
				throw LuaCallException( ("Global not found: " + std::string(tableName) ).c_str() );
			}
		
			//Look for function in that table
			lua_getfield(LUA_STATE, -1, functionName.c_str() );
			if(!lua_isfunction(LUA_STATE, -1)){
				lua_settop (LUA_STATE, 0);
				throw LuaCallException( (std::string(functionName) + " is not a function in " + tableName).c_str() );
			}
		
			//Push arguments onto the Lua stack
			LuaStack::pushStack<_ArgTypes...>(LUA_STATE, arguments...);
		
			//Perform function call
			if (lua_pcall(LUA_STATE, static_cast<int>(TemplateUtil::GetNrOfArguments<_ArgTypes...>()), 0, 0) != 0)
					throw LuaCallException(lua_tostring(LUA_STATE, -1));
		}
	};
    
    template<typename _RetType, typename... _ArgTypes>
    _RetType LuaScript::CallFunction(const char* fnName, _ArgTypes... args)
    {
        return LuaScript::Call<_RetType>::LuaFunction(fnName, args...);
    }
    
    template<typename _RetType, typename... _ArgTypes>
    _RetType LuaScript::CallMethod(const char* className, const char* fnName, _ArgTypes... args)
    {
        return LuaScript::Call<_RetType>::LuaStaticMethod(className, fnName, args...);
    }

	#undef LUA_STATE
}