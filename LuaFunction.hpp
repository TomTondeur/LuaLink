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

#include <lua.hpp>
#include <map>
#include <vector>
#include <string>

namespace LuaLink
{
	template<typename T> struct LuaStaticMethod;
	template<typename T> struct LuaMethod;
	template<typename T> class	LuaClass;

	struct LuaFunction
	{
		template<typename FunctionType>
		// // Add a C++ global function to the Lua environment
		static void Register(FunctionType pFunc, const std::string& name);
        
        static int DefaultErrorHandling(lua_State* L, int narg);
		
	private:	
		template<typename T> friend struct LuaStaticMethod; //LuaStaticMethod uses the same function table as LuaFunction
		friend class LuaScript; //LuaScript needs access to Commit, rather befriend LuaScript than expose Commit to everything
		template<typename T> friend class LuaClass;
		template<typename T> friend struct LuaMethod;
	
		//Struct form of wrapper/callbacks, necessary to keep a lookup table of all wrappers/callbacks
		struct Unsafe_LuaFunc;
	
		//Contains all registered global functions, is flushed after functions are pushed to Lua environment
		static std::map<std::string, std::vector<Unsafe_LuaFunc> > s_LuaFunctionMap;
	
		//* Contains all registered functions & static member functions
		//* Is filled when functions are pushed to Lua environment
		//* Used to retrieve callbacks on Lua function calls
		static std::vector<Unsafe_LuaFunc> s_LuaFunctionTable;

		template<typename _RetType, typename... _ArgTypes>
		// // Add a C++ member function to the appropriate lookup table
		static void Register_Impl(_RetType(*pFunc)(_ArgTypes...), const std::string& name);
	
		// // Pushes all registered member functions to the Lua environment
		static void Commit(lua_State* pLuaState);

		// // Throws out all references to functions that are left over from previous commits
		static void Release(void);

		static int OverloadedErrorHandling(lua_State* L, int narg);

		// CALLBACK WRAPPERS
	
		// Tries out all overloads until it finds an overload that matches the arguments used in the Lua call
		static int LuaFunctionDispatch(lua_State* L);

		//Disable default constructor, destructor, copy constructor & assignment operator
		LuaFunction(void);
		~LuaFunction(void);
		LuaFunction(const LuaFunction& src);
		LuaFunction& operator=(const LuaFunction& src);
	};
    
    namespace detail {
        typedef int(*ArgErrorCbType)(lua_State*, int);
        
        typedef int(*WrapperDoubleArg)(lua_State*, void*, ArgErrorCbType);
        typedef int(*WrapperSingleArg)(lua_State*);
        
        template<typename _RetType, typename... _ArgTypes> struct FunctionWrapper;
    }
}

#include "LuaFunction.inl"
