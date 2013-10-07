#pragma once
#include "../LuaFunction/LuaFunction.h"

namespace LuaLink
{
	template<typename T> class LuaClass;
	template<typename T> struct LuaMethod;

	template<typename ClassT>
	struct LuaStaticMethod
	{
		template<typename FunctionType>
		static void Register(FunctionType pFunc, const std::string& name);

	private:
		friend class LuaClass<ClassT>;	
		friend struct LuaMethod<ClassT>;

		static std::map<std::string, std::vector<LuaFunction::Unsafe_LuaFunc> > s_LuaFunctionMap;
	
		static int(*s_OverloadedConstructorWrapper)(lua_State*, LuaFunction::WrapperDoubleArg, void*, LuaFunction::ArgErrorCbType onArgError);
	
		template<typename _RetType, typename... _ArgTypes>
		static void Register_Impl(_RetType(*pFunc)(_ArgTypes...), const std::string& name);
	
		static void Commit(lua_State* pLuaState, int metatable);
		static void CommitConstructors(lua_State* pLuaState, int metatable, lua_CFunction ctorWrapper, int(*overloadedCtorWrapper)(lua_State*, LuaFunction::WrapperDoubleArg, void*, LuaFunction::ArgErrorCbType onArgError));

		static int OverloadedCTorDispatch(lua_State* L);
		
		//Disable default constructor, destructor, copy constructor & assignment operator
		LuaStaticMethod(void);
		~LuaStaticMethod(void);
		LuaStaticMethod(const LuaStaticMethod& src);
		LuaStaticMethod& operator=(const LuaStaticMethod& src);
	};
}
#include "LuaStaticMethod.inl"
