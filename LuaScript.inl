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
#include <map>

#include "luadbg.h"
#include "LuaStack.h"
#include "LuaClass.h"
#include "LuaAuto.hpp"

namespace LuaLink
{
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
			if (lua_pcall(LUA_STATE, sizeof...(_ArgTypes), 1, 0) != 0)
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
			if (lua_pcall(LUA_STATE, sizeof...(_ArgTypes), 1, 0) != 0)
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
			if (lua_pcall(LUA_STATE, sizeof...(_ArgTypes), 0, 0) != 0)
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
			if (lua_pcall(LUA_STATE, sizeof...(_ArgTypes), 0, 0) != 0)
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
    
#ifdef LUALINK_DEFINE
    //Initialize static members
    std::unique_ptr<lua_State> LuaScript::s_pLuaState;
    
    //Constructor & destructor
    
    LuaScript::LuaScript(const std::string& filename) : m_Filename(filename), InitializeEnvironment(nullptr) {}
    LuaScript::~LuaScript(void) {}
    
    //Methods
    
    // // Opens and loads the file linked to this object
    void LuaScript::Load(void(*initializeEnvironmentFn)(void), bool bOpenLibs, bool bResetState)
    {
        InitializeEnvironment = initializeEnvironmentFn; //Set initializer callback
        
        //Allocate new lua_State if necessary
        if(bResetState || !s_pLuaState){
            s_pLuaState = std::unique_ptr<lua_State>(lua_newstate(&LuaAllocate, nullptr)); //Unique pointer automatically destroys previous lua_State
            
            if(!s_pLuaState)
                throw LuaLoadException("Error allocating new lua state");
        }
        
        //Opens commonly used libraries
        if(bOpenLibs)
            luaL_openlibs(LUA_STATE);
        
        // Load a Lua script chunk without executing it
        switch(luaL_loadfile(s_pLuaState.get(), m_Filename.c_str() ))
        {
            case 0:
                break;
            case LUA_ERRFILE:	// Unable to open the file from luaL_loadfile()
            case LUA_ERRSYNTAX: // Syntax error in the lua code in the file from lua_load()
            case LUA_ERRMEM:	// Memory allocation error from lua_load()
                throw LuaLoadException(lua_tostring(LUA_STATE, -1));
                break;
            default:
                throw LuaLoadException("An unknown error has occured while loading file " + m_Filename);
        }
    }
    
    // // Adds all registered C++ functions and classes to the environment and performs an initial run
    void LuaScript::Initialize(void)
    {
        if(LUA_STATE == nullptr)
            Load();
        
        LuaFunction::Release(); //Release any leftover functions
        
        LuaAutoFunction::RegisterAll();
        LuaAutoClass::RegisterAll();
        
        if (InitializeEnvironment)
            InitializeEnvironment();
        
        LuaFunction::Commit(LUA_STATE); //Commit all functions registered in 'InitializeEnvironment'
        
        //Runs the script a first time to register functions and classes declared in the Lua script
        switch(lua_pcall(LUA_STATE, 0, LUA_MULTRET, 0))
        {
            case 0:
                break;
            case LUA_ERRRUN:
            case LUA_ERRMEM:
            case LUA_ERRERR:
                throw LuaCallException(lua_tostring(LUA_STATE, -1));
                break;
            default:
                throw LuaCallException("An unknown error has occured while executing file " + m_Filename);
        }
    }
    
    //  The type of the memory-allocation function used by Lua states. The allocator function
    //  must provide a functionality similar to realloc, but not exactly the same. Its arguments
    //  are:
    //     ud, an opaque pointer passed to lua_newstate;
    //     ptr, a pointer to the block being allocated/reallocated/freed;
    //     osize, the original size of the block;
    //     nsize, the new size of the block.
    //
    //  ptr is NULL if and only if osize is zero. When nsize is zero, the allocator
    //  must return NULL; if osize is not zero, it should free the block pointed to by ptr. When nsize
    //  is not zero, the allocator returns NULL if and only if it cannot fill the request. When nsize
    //  is not zero and osize is zero, the allocator should behave like malloc. When nsize and osize
    //  are not zero, the allocator behaves like realloc().
    //
    //  Lua assumes that the allocator never fails when osize >= nsize.
    //
    //  See also this documentation http://www.lua.org/manual/5.2/manual.html#lua_Alloc
    //
    void* LuaScript::LuaAllocate(void *ud, void *ptr, size_t osize, size_t nsize)
    {
        void *pOut = nullptr;
        
        if (osize && nsize && ptr) {
            if (osize < nsize)
                pOut = realloc (ptr, nsize);
            else
                pOut = ptr;
        }
        else if (nsize == 0)
            free(ptr);
        else
            pOut = malloc (nsize);
        
        return pOut;
    }
    
    lua_State* LuaScript::GetLuaState(void)
    {
        return LUA_STATE;
    }
#endif //LUALINK_DEFINE

	#undef LUA_STATE
}
