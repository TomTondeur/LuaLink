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

#include "LuaMethod.h"
#include "LuaStaticMethod.h"
#include "LuaVariable.h"
#include "LuaScript.h"

namespace LuaLink
{
    template<typename T>
    struct cast_from {
        template<typename U>
        static U static__cast(const T& t)
        {
            return static_cast<U>(t);
        }
        
        template<typename U>
        static U reinterpret__cast(const T& t)
        {
            return reinterpret_cast<U>(t);
        }
        
        template<typename U>
        static U dynamic__cast(const T& t)
        {
            return dynamic_cast<U>(t);
        }
    };

	template <typename T>
	std::string LuaClass<T>::s_ClassName = std::string();
    
    template <typename T>
    void(*LuaClass<T>::s_fn_inst_reg)(T*) = nullptr;

	template <typename T>
	// // Registers Class T in the Lua environment
	void LuaClass<T>::Register(const std::string& className, bool bAllowInheritance)
	{
        Register(className, bAllowInheritance, T::RegisterStaticsAndMethods,
                 cast_from<void(*)(T*)>::template reinterpret__cast<void(*)(void*)>(T::RegisterVariables));
	}
    
    template<typename T>
    void LuaClass<T>::Register(const std::string& className, bool bAllowInheritance, void(*fn_static_reg)(void), void(*fn_inst_reg)(void*))
    {
        s_ClassName = className;
        s_fn_inst_reg = reinterpret_cast<void(*)(T*)>(fn_inst_reg);
        
        lua_State* L = LuaScript::GetLuaState();
        
        LuaVariable::Commit(L); //Flush any global variables that may be registered, just in case
        
        //Create new table
        lua_newtable(L);
        
        //Push metamethods
        //
        lua_pushstring(L, "__gc");
        lua_pushcfunction(L, gc_obj);
        lua_settable(L, -3);
        
        lua_pushstring(L, "__tostring");
        lua_pushcfunction(L, to_string);
        lua_settable(L, -3);
        
        //self.__index = self => when used as metatable, missing identifiers will be looked up in this table
        lua_pushstring(L,"__index");
        lua_pushvalue(L,-2);
        lua_settable(L,-3);
        
        //enable/disable inheritance
        lua_pushstring(L, "inherit");
        lua_pushcfunction(L, bAllowInheritance ? returnDerived : noInheritance);
        lua_settable(L,-3);
        
        T::RegisterStaticsAndMethods();
        
        LuaStaticMethod<T>::CommitConstructors(L, -3, ConstructorWrapper, ConstructorWrapper);
        LuaStaticMethod<T>::Commit(L, -3);
        LuaMethod<T>::Commit(L, -3);
        LuaVariable::Commit(L);
        
        //Set table name (pops table)
        lua_setglobal(L, s_ClassName.c_str());
    }
    
	
	template <typename T>
	int LuaClass<T>::ConstructorWrapper(lua_State * L)
	{
		return ConstructorWrapper(L, 
							reinterpret_cast<detail::WrapperDoubleArg>(LuaStack::getVariable<void*>( L, lua_upvalueindex(1) ) ),
							LuaStack::getVariable<void*>( L, lua_upvalueindex(2) ),
							LuaFunction::DefaultErrorHandling);
	}

	template <typename T>
	int LuaClass<T>::ConstructorWrapper(lua_State * L, detail::WrapperDoubleArg pWrapper, void* pFunc, detail::ArgErrorCbType onArgError)
	{
		//Create new object
		if(pWrapper(L, pFunc, onArgError) == -1)
			return onArgError(L, 0);
	
		T*  pObj = static_cast<T*>( LuaStack::getVariable<void*>( L, -1) );
	
		lua_newtable(L); //Create new table

		//Add core_ entry to the table
		lua_pushstring(L,"core_");		
		T** memLoc = static_cast<T**>(lua_newuserdata(L, sizeof(T *))); // Push new userdata value
		*memLoc = pObj; //Userdata should point to our newly allocated object
		lua_settable(L,-3);

		//Push nonstatic properties
        T::RegisterVariables(pObj);
		LuaVariable::Commit(L,-3);

		//Set the class table as metatable for this object
		lua_getglobal(L, s_ClassName.c_str());
		lua_setmetatable(L, -2);
	
		return 1; //Return 1 value, our new table
	}

	template <typename T>
	//Metamethod, called when garbage collector gets rid of our object
	int LuaClass<T>::gc_obj(lua_State * L)
	{
		//Retrieve C++ object
		lua_pushstring(L, "core_");
		lua_rawget(L, 1);
		T** ppObj = static_cast < T ** >(lua_touserdata(L, -1));
		
		//Delete object
		if(ppObj)
			delete *ppObj;
		
		return 0; //No return value
	}	

	template <typename T>
	//Metamethod, called when converting our object to a string
	int LuaClass<T>::to_string(lua_State* L)
	{
		lua_pushstring(L,"core_");
		lua_rawget(L,1);
		T** ppObj = static_cast<T**>(lua_touserdata(L, -1));
		
		if(ppObj)
			lua_pushfstring(L, "%s (%p)", s_ClassName.c_str(), (void*)*ppObj);
		else
			lua_pushfstring(L,"Empty %s object", s_ClassName.c_str());
		
		return 1; //Return 1 string
	}

	template<typename T>
	int LuaClass<T>::returnDerived(lua_State* L)
	{
		//Create new empty table
		lua_newtable(L);
	
		//Transfer metamethods
		//
		lua_pushstring(L,"__gc");
		lua_pushcfunction(L, gc_obj);
		lua_settable(L,-3);
		
		lua_pushstring(L, "__tostring");
		lua_pushcfunction(L, to_string);
		lua_settable(L, -3);
	
		//self.__index = self => when used as metatable, missing identifiers will be looked up in this table
		lua_pushstring(L,"__index");
		lua_pushvalue(L,-2);
		lua_settable(L,-3);

		//Set our class as metatable for the new object
		lua_getglobal(L,s_ClassName.c_str());
		lua_setmetatable(L,-2);

		return 1;
	}

	template<typename T>
	int LuaClass<T>::noInheritance(lua_State* L)
	{
		return luaL_error(L, "It is illegal to inherit from this class");
	}
}
