#pragma once

#include <lua.hpp>
#include <functional>
#include <memory>

#include <map>

template<>
//Specify policy to release lua_State*
struct std::default_delete<lua_State>{
	void operator()(lua_State* ptr){
		if(ptr)
			lua_close(ptr);
	}
};

namespace LuaLink
{
	template<typename T>class LuaClass;

	class LuaScript
	{
	public:
		//Constructor & destructor

		LuaScript(const ::std::string& filename);
		virtual ~LuaScript(void);

		//Methods

		// // Opens and loads the file linked to this object
		void Load(void(*initializeEnvironmentFn)(void) = nullptr, bool bOpenLibs = true, bool bResetState = false);
		// // Adds all registered C++ functions and classes to the environment and performs an initial run
		void Initialize (void);

		template<typename _RetType>
		struct Call;
	
	private:
		template<typename T> friend class LuaClass;
	
		//Returns lua_State* (to use when commiting classes)
		static lua_State* GetLuaState(void);

		//Custom Lua allocator
		static void* LuaAllocate(void *ud, void *ptr, size_t osize, size_t nsize);
	
		//Datamembers
	
		::std::string m_Filename;
		void(*InitializeEnvironment)(void); //Function where all needed variables/functions/classes are registeredd to the lua_State

		static ::std::unique_ptr<lua_State> s_pLuaState;

		//Disabling default copy ConstructorWrapper & assignment operator
		LuaScript(const LuaScript& src);
		LuaScript& operator=(const LuaScript& src);
	};
}
	#include "LuaScript.inl"
