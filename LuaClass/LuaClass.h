#pragma once

#include "../LuaFunction/LuaFunction.h"

namespace LuaLink
{
	template <typename T> 
	class LuaClass 
	{
	  public:
		// // Registers Class T in the Lua environment
		static void Register(const std::string& className, bool bAllowInheritance = true);
	
	private:
		// // This is the name the class is registered with in Lua
		static std::string s_ClassName;

		// // Creates new object in C++ and pushes it to the Lua stack
		static int ConstructorWrapper(lua_State * L);
		static int ConstructorWrapper(lua_State * L, LuaFunction::WrapperDoubleArg pWrapper, void* cb, LuaFunction::ArgErrorCbType onArgError);

		// // Metamethod, called when garbage collector gets rid of our object
		static int gc_obj(lua_State * L);	

		// // Metamethod, called when converting our object to a string
		static int to_string(lua_State* L);

		// // Returns new table that derives from the table linked to this class
		static int returnDerived(lua_State* L);

		// // Disables inheritance for this class
		static int noInheritance(lua_State* L);

		//Disable default constructor, destructor, copy constructor & assignment operator
		LuaClass(void);
		~LuaClass(void);
		LuaClass(const LuaClass& src);
		LuaClass& operator=(const LuaClass& src);
	};
}
	#include "LuaClass.inl"
