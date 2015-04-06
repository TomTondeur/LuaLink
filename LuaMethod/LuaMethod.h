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

namespace LuaLink
{
    template<typename T> class LuaClass;
    
    namespace detail {
        // Callback wrappers
        template<typename ClassT, typename _RetType, typename... _ArgTypes> struct MethodWrapper;
    }

	template<typename ClassT>
	struct LuaMethod
	{	
		template<typename MemberFunctionType>
		// // Add a C++ member function to the Lua environment
		static void Register(MemberFunctionType pFunc, const std::string& name);

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
		static std::map<std::string, std::vector<Unsafe_MethodWrapper> > s_LuaFunctionMap;

		//* Contains all registered member functions
		//* Is filled when functions are pushed to Lua environment
		//* Used to retrieve callbacks on Lua function calls
		static std::vector<Unsafe_MethodWrapper> s_LuaFunctionTable;
	
		template<typename _RetType, typename... _ArgTypes>
		// // Add a C++ member function to the appropriate lookup table
		static void Register_Impl(_RetType(ClassT::*pFunc)(_ArgTypes...), const std::string& name);
	
		// // Pushes all registered member functions to the Lua environment
		static void Commit(lua_State* pLuaState, int tablePosOnStack);
	
		// // Retrieves the this pointer from the table on the bottom of the stack
		static void PushThisPointer(lua_State* L);

		// // Tries out all overloads until it finds an overload that matches the arguments used in the Lua call
		static int OverloadDispatch(lua_State* L);
		
		// // Common code in all MethodWrappers, returns pointer to pointer to object to call member function on
		static ClassT** GetObjectAndVerifyStackSize(lua_State* L, int nrOfArgs);
	
		//Disable default constructor, destructor, copy constructor & assignment operator
		LuaMethod(void);
		~LuaMethod(void);
		LuaMethod(const LuaMethod& src);
		LuaMethod& operator=(const LuaMethod& src);
	};

    namespace detail {
        //TODO: Move to .inl file
        template<typename ClassT>struct MethodWrapper<ClassT, void>
        {
            static int execute(lua_State* pLuaState, typename LuaMethod<ClassT>::Unsafe_MethodType fn, ArgErrorCbType onArgError)
            {
                auto ppObj = LuaMethod<ClassT>::GetObjectAndVerifyStackSize(pLuaState, 0);
                if(ppObj == nullptr)
                    return onArgError(pLuaState, 0);
                
                ((*ppObj)->*(fn))();
                return 0;
            }
            
            static int execute(lua_State* pLuaState)
            {
                return execute(pLuaState,
                               LuaMethod<ClassT>::s_LuaFunctionTable[static_cast<unsigned int>(lua_tointeger( pLuaState, lua_upvalueindex(1) ) )].pFunc,
                               LuaFunction::DefaultErrorHandling);
            }
        };
    }
}

#include "LuaMethod.inl"
