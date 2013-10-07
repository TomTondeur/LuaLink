#pragma once
#include <vector>
#include <lua.hpp>

namespace LuaLink
{
	template <typename T> class LuaClass;

	struct LuaVariable
	{
		template<typename T>
		static void Register(T& var, const char* varName);

	private:
		friend class LuaScript;
		template<typename T> friend class LuaClass;

		struct Unsafe_VariableWrapper;

		static std::vector<Unsafe_VariableWrapper> s_VariablesToCommit;

		static void Commit(lua_State* pLuaState, int tableIdx = 0);

		template<typename T>
		struct Implementation
		{
			static int get(lua_State* L);

			static int set(lua_State* L);
		};
		
		//Disable default constructor, destructor, copy constructor & assignment operator
		LuaVariable(void);
		~LuaVariable(void);
		LuaVariable(const LuaVariable& src);
		LuaVariable& operator=(const LuaVariable& src);
	};
}
	#include "LuaVariable.inl"
