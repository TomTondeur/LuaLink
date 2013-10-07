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
		
	private:	
		template<typename T> friend struct LuaStaticMethod; //LuaStaticMethod uses the same function table as LuaFunction
		friend class LuaScript; //LuaScript needs access to Commit, rather befriend LuaScript than expose Commit to everything
		template<typename T> friend class LuaClass;
		template<typename T> friend struct LuaMethod;
	
		typedef int(*ArgErrorCbType)(lua_State*, int);

		typedef int(*WrapperDoubleArg)(lua_State*, void*, ArgErrorCbType);
		typedef int(*WrapperSingleArg)(lua_State*);
	
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

		static int DefaultErrorHandling(lua_State* L, int narg);
		static int OverloadedErrorHandling(lua_State* L, int narg);

		// CALLBACK WRAPPERS
	
		// Tries out all overloads until it finds an overload that matches the arguments used in the Lua call
		static int LuaFunctionDispatch(lua_State* L);

		template<typename _RetType, typename... _ArgTypes> struct FunctionWrapper;
	
		//no ret, 0 arg
		template<>struct FunctionWrapper<void>
		{
			typedef void(*CbType)(void);

			static int execute(lua_State* pLuaState, void* fn, ArgErrorCbType onArgError)
			{
				if(lua_gettop(pLuaState) != 0) //argc
					return onArgError(pLuaState, 0);

				static_cast<CbType>(fn)();
				return 0;
			}

			static int execute(lua_State* pLuaState){return execute(pLuaState, lua_touserdata( pLuaState, lua_upvalueindex(1) ), DefaultErrorHandling);}
		};
	
		//Disable default constructor, destructor, copy constructor & assignment operator
		LuaFunction(void);
		~LuaFunction(void);
		LuaFunction(const LuaFunction& src);
		LuaFunction& operator=(const LuaFunction& src);
	};
}
	#include "LuaFunction.inl"
