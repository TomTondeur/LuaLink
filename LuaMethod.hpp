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

#include "TemplateUtil.h"

namespace LuaLink
{
    template<typename T> class LuaClass;
    
    namespace detail {
        // Callback wrappers
        template<typename ClassT, typename _RetType, typename... _ArgTypes> struct MethodWrapper;
    }

	template<typename ClassT>
	class LuaMethod
	{
    public:
		template<typename _RetType, typename... _ArgTypes>
        // // Add a C++ member function to the appropriate lookup table
        static void Register(_RetType(ClassT::*pFunc)(_ArgTypes...), const char* name);
        
	private:
        template<typename T, typename _RetType, typename... _ArgTypes>
        friend class detail::MethodWrapper;
		friend class LuaClass<ClassT>; //LuaClass<ClassT> needs access to Commit, rather befriend LuaClass<ClassT> than expose Commit to everything
	
		typedef void(ClassT::*Unsafe_MethodType)();
		typedef int(*WrapperDoubleArg)(lua_State*, Unsafe_MethodType, detail::ArgErrorCbType);
		typedef int(*WrapperSingleArg)(lua_State*);
	
		//Struct form of wrapper/callbacks, necessary to keep a lookup table of all wrappers/callbacks
		struct Unsafe_MethodWrapper;
	
		//Contains all registered member functions, is flushed after functions are pushed to Lua environment
		static std::map<const char*, std::vector<Unsafe_MethodWrapper>, detail::CStrCmp > s_LuaFunctionMap;

		//* Contains all registered member functions
		//* Is filled when functions are pushed to Lua environment
		//* Used to retrieve callbacks on Lua function calls
		static std::vector<Unsafe_MethodWrapper> s_LuaFunctionTable;
	
		// // Pushes all registered member functions to the Lua environment
		static void Commit(lua_State* pLuaState, int tablePosOnStack);
	
		// // Retrieves the this pointer from the table on the bottom of the stack
		static void PushThisPointer(lua_State* L);

		// // Tries out all overloads until it finds an overload that matches the arguments used in the Lua call
		static int OverloadDispatch(lua_State* L);
		
		// // Common code in all MethodWrappers, returns pointer to pointer to object to call member function on
		static ClassT** GetObjectAndVerifyStackSize(lua_State* L, int nrOfArgs);
	
		//Disable default constructor, destructor, copy constructor & assignment operator
		LuaMethod(void) = delete;
		~LuaMethod(void) = delete;
		LuaMethod(const LuaMethod& src) = delete;
		LuaMethod& operator=(const LuaMethod& src) = delete;
	};
}

#include "LuaMethod.inl"
