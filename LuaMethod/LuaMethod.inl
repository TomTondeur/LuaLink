#pragma once

#include "../LuaStack/LuaStack.h"
#include <vector>
#include <map>

namespace LuaLink
{
	template<typename ClassT>
	template<typename MemberFunctionT>
	// // Add a C++ member function to the Lua environment
	void LuaMethod<ClassT>::Register(MemberFunctionT pFunc, const std::string& name){
		Register_Impl(pFunc, name);
	}
	
	template<typename ClassT>
	//Struct form of wrapper/callbacks, necessary to keep a lookup table of all wrappers/callbacks
	struct LuaMethod<ClassT>::Unsafe_MethodWrapper{
		Unsafe_MethodWrapper(WrapperDoubleArg w1, WrapperSingleArg w2, Unsafe_MethodType cb) : pWrapper(w1), pWrapperSingle(w2), pFunc(cb){}

		WrapperDoubleArg pWrapper; //Used for calls to overloaded member functions
		WrapperSingleArg pWrapperSingle; //Used for calls to non-overloaded member functions
		Unsafe_MethodType pFunc; //Serves as callback, discards return & argument types, wrappers restore return & argument types
	};

	template<typename ClassT>
	template<typename _RetType, typename... _ArgTypes>
	// // Add a C++ member function to the appropriate lookup table
	void LuaMethod<ClassT>::Register_Impl(_RetType(ClassT::*pFunc)(_ArgTypes...), const std::string& name)
	{
		//Get iterator that points to the lookup table associated with 'name'
		auto it = s_LuaFunctionMap.find(name);
		if( it == s_LuaFunctionMap.end() )
			it = s_LuaFunctionMap.insert(make_pair(name, std::vector<Unsafe_MethodWrapper>() ) ).first;

		//Add wrapper to lookup table
		it->second.push_back(Unsafe_MethodWrapper(MethodWrapper<_RetType, _ArgTypes...>::execute, MethodWrapper<_RetType, _ArgTypes...>::execute, reinterpret_cast<Unsafe_MethodType>(pFunc)));
	}

	template<typename ClassT>
	// // Pushes all registered member functions to the Lua environment
	void LuaMethod<ClassT>::Commit(lua_State* pLuaState, int tablePosOnStack)
	{
		if(!s_LuaFunctionTable.empty())
			s_LuaFunctionTable.clear();

		for(auto& elem : s_LuaFunctionMap)
		{
			//No overloading
			if(elem.second.size() == 1){
				s_LuaFunctionTable.push_back(elem.second[0]); //Add function to the lookup table

				lua_pushstring(pLuaState, elem.first.c_str() ); //Push function name

				//Push index of function in lookup table & wrapper as closure
				lua_pushunsigned(pLuaState, s_LuaFunctionTable.size() - 1);
				lua_pushcclosure(pLuaState, elem.second[0].pWrapperSingle, 1);
				
				lua_settable(pLuaState, tablePosOnStack); //Add entry to the Lua table
				continue;
			}

			//This function needs to be overloaded =>

			//Move functions to lookup table
			unsigned int startIdx = s_LuaFunctionTable.size();
			unsigned int endIdx = startIdx + elem.second.size();

			std::move(elem.second.begin(), elem.second.end(), std::insert_iterator<std::vector<Unsafe_MethodWrapper>>(s_LuaFunctionTable, s_LuaFunctionTable.end()));
		
			lua_pushstring(pLuaState, elem.first.c_str()); //Push function name

			//Push start and end indices of functions in lookup table (endIdx is one past last function, like .end() iterators)
			lua_pushunsigned(pLuaState, startIdx);
			lua_pushunsigned(pLuaState, endIdx);

			lua_pushcclosure(pLuaState, OverloadDispatch, 2); //Push closure

			lua_settable(pLuaState, tablePosOnStack); //Set table
		}
		s_LuaFunctionMap.swap(std::map<std::string, std::vector<Unsafe_MethodWrapper>>());
	}

	template<typename ClassT>
	void LuaMethod<ClassT>::PushThisPointer(lua_State* L)
	{
		//Make sure the object was pushed to the stack
		if(lua_istable(L, 1) == 0)
			luaL_error(L, "Calling a nonstatic member function requires a reference to an object");

		//Push ppObj to stack
		lua_pushstring(L, "core_");
		lua_rawget(L, 1); 
	
		if(lua_isuserdata(L, -1) == 0)
			luaL_error(L, "Calling a nonstatic member function requires a reference to an object");
	
		lua_remove(L, 1); //Remove 'self table' from stack
	}
	
	template<typename ClassT>
	// Tries out all overloads until it finds an overload that matches the arguments used in the Lua call
	int LuaMethod<ClassT>::OverloadDispatch(lua_State* L)
	{
		//Retrieve range in lookup table where overloads are located
		unsigned int startIdx = lua_tounsigned( L, lua_upvalueindex(1) );
		unsigned int endIdx =	lua_tounsigned( L, lua_upvalueindex(2) );

		PushThisPointer(L);

		//Try all functions
		for(unsigned int i = startIdx; i < endIdx; ++i){
			int ret = s_LuaFunctionTable[i].pWrapper(L, s_LuaFunctionTable[i].pFunc, LuaFunction::OverloadedErrorHandling);
		
			if(ret < 0)
				continue;
		
			return ret;
		}

		//No correct overload found
		return luaL_error(L, "Invalid member function call - no overload found that takes these parameters.");
	}

	template<typename ClassT>
	// // Common code in all MethodWrappers, returns pointer to pointer to object to call member function on
	ClassT** LuaMethod<ClassT>::GetObjectAndVerifyStackSize(lua_State* L, int nrOfArgs)
	{
		int argc = lua_gettop(L);
		if(argc != nrOfArgs + 1) //stack should contain 'this pointer' + args
			return nullptr;
	
		auto ppObj = static_cast<ClassT**>(lua_touserdata(L, -1) ); //Retrieve internal object
		lua_pop(L, 1); //Remove internal object from stack
		return ppObj; //Return internal object
	}
	
	// CALLBACK WRAPPERS

	#define GET_THIS(ARGC) auto ppObj = GetObjectAndVerifyStackSize(pLuaState, ARGC); if(!ppObj) return onArgError(pLuaState, 0); bool isOk = true;
	#define GET_ARG(ARGN) auto arg##ARGN = LuaStack::getVariable<_Arg##ARGN##Type>(pLuaState, ARGN, isOk); if(!isOk) return onArgError(pLuaState, ARGN);
	#define DO_LUACALLBACK(CBTYPE,...) ((*ppObj)->*(reinterpret_cast<CBTYPE>(fn)))(##__VA_ARGS__)

	#define EXECUTE_V2 static int execute(lua_State* L){ \
		PushThisPointer(L);\
		return execute(L, s_LuaFunctionTable[static_cast<unsigned int>(lua_tounsigned( L, lua_upvalueindex(1) ) )].pFunc, LuaFunction::DefaultErrorHandling);}

	//no ret, 1 arg
	template<typename ClassT>
	template<typename _Arg1Type>
	struct LuaMethod<ClassT>::MethodWrapper<void, _Arg1Type>
	{
		static int execute(lua_State* pLuaState, Unsafe_MethodType fn, LuaFunction::ArgErrorCbType onArgError)
		{
			GET_THIS(1)
			GET_ARG(1)
			DO_LUACALLBACK(void(ClassT::*)(_Arg1Type),arg1);
			return 0;
		}
		
		EXECUTE_V2
	};

	//no ret, 2 arg
	template<typename ClassT>
	template<typename _Arg1Type, typename _Arg2Type>
	struct LuaMethod<ClassT>::MethodWrapper<void, _Arg1Type, _Arg2Type>
	{
		static int execute(lua_State* pLuaState, Unsafe_MethodType fn, LuaFunction::ArgErrorCbType onArgError)
		{
			GET_THIS(2)
			GET_ARG(1)
			GET_ARG(2)
			DO_LUACALLBACK(void(ClassT::*)(_Arg1Type, _Arg2Type), arg1, arg2);		
			return 0;
		}
	
		EXECUTE_V2
	};

	//	 ret, 0 arg
	template<typename ClassT>
	template<typename _RetType>
	struct LuaMethod<ClassT>::MethodWrapper<_RetType>
	{
		static int execute(lua_State* pLuaState, Unsafe_MethodType fn, LuaFunction::ArgErrorCbType onArgError)
		{
			GET_THIS(0)
			LuaStack::pushVariable(	pLuaState, DO_LUACALLBACK( _RetType(ClassT::*)(void) ) );
			return 1;
		}
		
		EXECUTE_V2
	};

	//	 ret, 1 arg
	template<typename ClassT>
	template<typename _RetType, typename _Arg1Type>
	struct LuaMethod<ClassT>::MethodWrapper<_RetType, _Arg1Type>
	{
		static int execute(lua_State* pLuaState, Unsafe_MethodType fn, LuaFunction::ArgErrorCbType onArgError)
		{
			GET_THIS(1)
			GET_ARG(1)
			LuaStack::pushVariable(	pLuaState, DO_LUACALLBACK(_RetType(ClassT::*)(_Arg1Type), arg1) );
			return 1;
		}
		
		EXECUTE_V2
	};

	//	 ret, 2 arg
	template<typename ClassT>
	template<typename _RetType, typename _Arg1Type, typename _Arg2Type>
	struct LuaMethod<ClassT>::MethodWrapper<_RetType, _Arg1Type, _Arg2Type>
	{
		static int execute(lua_State* pLuaState, Unsafe_MethodType fn, LuaFunction::ArgErrorCbType onArgError)
		{
			GET_THIS(2)
			GET_ARG(1)
			GET_ARG(2)
			LuaStack::pushVariable(	pLuaState, DO_LUACALLBACK(_RetType(ClassT::*)(_Arg1Type, _Arg2Type), arg1, arg2) );
			return 1;
		}
		
		EXECUTE_V2
	};

	template<typename ClassT>
	std::vector<typename LuaMethod<ClassT>::Unsafe_MethodWrapper> LuaMethod<ClassT>::s_LuaFunctionTable = std::vector<typename LuaMethod<ClassT>::Unsafe_MethodWrapper>();

	template<typename ClassT>
	std::map<std::string, std::vector<typename LuaMethod<ClassT>::Unsafe_MethodWrapper> > LuaMethod<ClassT>::s_LuaFunctionMap = std::map<std::string, std::vector<typename LuaMethod<ClassT>::Unsafe_MethodWrapper> >();

	#undef GET_THIS
	#undef GET_ARG
	#undef DO_LUACALLBACK
	#undef EXECUTE_V2
}