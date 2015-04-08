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

#include "LuaFunction.hpp"

namespace LuaLink
{
	template <typename T> 
	class LuaClass 
	{
	  public:
        // // Registers Class T in the Lua environment
        static void Register(const std::string& className, bool bAllowInheritance = true);
        static void Register(const std::string& className, bool bAllowInheritance, void(*fn_static_reg)(void), void(*fn_inst_reg)(void*));
	
	private:
		// // This is the name the class is registered with in Lua
		static std::string s_ClassName;
        
        //This function will be used to register instances of this class in Lua
        static void(*s_fn_inst_reg)(T*);

		// // Creates new object in C++ and pushes it to the Lua stack
		static int ConstructorWrapper(lua_State * L);
		static int ConstructorWrapper(lua_State * L, detail::WrapperDoubleArg pWrapper, void* cb, detail::ArgErrorCbType onArgError);

		// // Metamethod, called when garbage collector gets rid of our object
		static int gc_obj(lua_State * L);	

		// // Metamethod, called when converting our object to a string
		static int to_string(lua_State* L);

		// // Returns new table that derives from the table linked to this class
		static int returnDerived(lua_State* L);

		// // Disables inheritance for this class
		static int noInheritance(lua_State* L);

		//Disable default constructor, destructor, copy constructor & assignment operator
		LuaClass(void) = delete;
		~LuaClass(void) = delete;
		LuaClass(const LuaClass& src) = delete;
		LuaClass& operator=(const LuaClass& src) = delete;
	};
}

#include "LuaClass.inl"
