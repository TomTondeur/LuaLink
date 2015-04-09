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

#include <lua.hpp>
#include "LuaStack.hpp"

namespace LuaLink
{
    namespace detail {
        namespace LuaVariable {
            template<typename T>
            //Callbacks to get and set variables in Lua, comparable to auto-properties in C#
            struct Implementation
            {
                static int get(lua_State* L) {
                    lua_pushinteger(L, *static_cast<T*>(lua_touserdata(L, lua_upvalueindex(1))));
                    return 1;
                }
                static int set(lua_State* L) {
                    *static_cast<T*>(lua_touserdata(L, lua_upvalueindex(1))) = LuaStack::getVariable<T>(L, 1);
                    return 1;
                }
            };
            
            //Intermediate format to store variables as they wait to be commited to Lua
            struct Unsafe_VariableWrapper
            {
                template<typename T>
                Unsafe_VariableWrapper(T* pVar, const char* name) : Data(static_cast<void*>(pVar)),
                Name(name),
                Getter(Implementation<T>::get),
                Setter(Implementation<T>::set) {}
                void* Data;
                lua_CFunction Getter;
                lua_CFunction Setter;
                const char* Name;
            };
            
            extern void Register_Impl(Unsafe_VariableWrapper&&);
        }
    }

	template<typename T>
	// // Register a variable of type T, will be automatically registered for a class if done so in the appropriate member function
	void LuaVariable::Register(T& var, const char* varName)
	{
        detail::LuaVariable::Register_Impl(detail::LuaVariable::Unsafe_VariableWrapper(&var, varName));
	}
}

#ifdef LUALINK_DEFINE

#include <vector>

namespace LuaLink {
    namespace detail {
        namespace LuaVariable {
            //Temporary container for variables that haven't been commited yet
            std::vector<Unsafe_VariableWrapper>& VariablesToCommit() {
                static std::vector<Unsafe_VariableWrapper> s;
                return s;
            }
            
            void Register_Impl(Unsafe_VariableWrapper&& v) {
                VariablesToCommit().push_back(std::forward<Unsafe_VariableWrapper>(v));
            }
        }
    }

    void LuaVariable::Commit(lua_State* pLuaState, int tableIdx)
    {
        using namespace detail::LuaVariable;
        
        //For each variable
        for(auto& elem : VariablesToCommit())
        {
            //Push string to the stack when registering for a table, so we can call settable later
            if(tableIdx != 0)
                lua_pushstring(pLuaState, elem.Name);
            
            //Create new table to hold 'get' and 'set' methods
            lua_newtable(pLuaState);
            
            //Push getter
            lua_pushstring(pLuaState, "get");
            lua_pushlightuserdata(pLuaState, elem.Data);
            lua_pushcclosure(pLuaState, elem.Getter, 1);
            lua_settable(pLuaState, -3);
            
            //Push setter
            lua_pushstring(pLuaState, "set");
            lua_pushlightuserdata(pLuaState, elem.Data);
            lua_pushcclosure(pLuaState, elem.Setter, 1);
            lua_settable(pLuaState, -3);
            
            //Set as global or as field in a table
            if(tableIdx == 0)
                lua_setglobal(pLuaState, elem.Name);
            else
                lua_settable(pLuaState, tableIdx);
        }
        
        //Flush the cache of variables to commit so this can be re-used
        VariablesToCommit().clear();
    }
}
#endif //LUALINK_DEFINE

