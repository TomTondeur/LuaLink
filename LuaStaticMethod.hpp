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
	//Forward declarations
	template<typename T> class LuaClass;
	template<typename T> struct LuaMethod;

	template<typename ClassT>
	struct LuaStaticMethod
	{
		template<typename FunctionType>
		// //Registers a static method of class ClassT to use in Lua
		static void Register(FunctionType pFunc, const std::string& name);

	private:
		//LuaClass and LuaMethod need more 'intimate access than we want to expose to the end user
		friend class LuaClass<ClassT>;	
		friend struct LuaMethod<ClassT>;
		
		//Contains all registered static methods for class ClassT, is flushed after functions are pushed to Lua environment
		static std::map<std::string, std::vector<LuaFunction::Unsafe_LuaFunc> > s_LuaFunctionMap;
	
		//Will be called from OverloadedCTorDispatch when someone calls an overloaded constructor from Lua
		static int(*s_OverloadedConstructorWrapper)(lua_State*, detail::WrapperDoubleArg, void*, detail::ArgErrorCbType onArgError);
	
		template<typename _RetType, typename... _ArgTypes>
		// Register implementation
		static void Register_Impl(_RetType(*pFunc)(_ArgTypes...), const std::string& name);
	
		//Pushes all registered static methods to the provided lua_State*
		static void Commit(lua_State* pLuaState, int metatable);
		static void CommitConstructors(lua_State* pLuaState, int metatable, lua_CFunction ctorWrapper, int(*overloadedCtorWrapper)(lua_State*, detail::WrapperDoubleArg, void*, detail::ArgErrorCbType onArgError));

		//Will serve as callback from Lua when calling an overloaded constructor
		static int OverloadedCTorDispatch(lua_State* L);
		
		//Disable default constructor, destructor, copy constructor & assignment operator
		LuaStaticMethod(void);
		~LuaStaticMethod(void);
		LuaStaticMethod(const LuaStaticMethod& src);
		LuaStaticMethod& operator=(const LuaStaticMethod& src);
	};
}

#include "LuaStaticMethod.inl"
