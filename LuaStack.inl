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

namespace LuaLink
{
	//Partial specialization for LuaStack::Implementation_pushStack

	template<typename H, typename... T>
	struct LuaStack::Implementation_pushStack<H, T...>{
		static void pushStack(lua_State* pLua, H data, T... tailData) 
		{ 
			pushVariable<H>(pLua, data);
            pushStack(pLua, tailData...);
		}
	};
    
    template<typename T>
    struct LuaStack::Implementation_pushStack<T>{
        static void pushStack(lua_State* pLua, T data)
        {
            pushVariable<T>(pLua, data);
        }
    };
    
	template<>
	struct LuaStack::Implementation_pushStack<>{
		static void pushStack(lua_State* pLua) { }
	};

	template<typename... T>
	void LuaStack::pushStack(lua_State* pLua, T... data)
	{
		Implementation_pushStack<T...>::pushStack(pLua, data...);
	}
}

#ifdef LUALINK_DEFINE

#include <string>

namespace LuaLink {
    //getVariable (safe version)
    
    template<>
    std::string LuaStack::getVariable<std::string>(lua_State* pLua, int varIdx, bool& isOk)
    {
        const char* str = lua_tostring(pLua, varIdx);
        isOk = str != nullptr;
        
        return std::string(str);
    }
    
    template<>
    int LuaStack::getVariable<int>(lua_State* pLua, int varIdx, bool& isOk)
    {
        int isnum;
        lua_Integer i = lua_tointegerx(pLua, varIdx, &isnum);
        isOk = isnum != 0;
        
        return static_cast<int>(i);
    }
    
    template<>
    unsigned int LuaStack::getVariable<unsigned int>(lua_State* pLua, int varIdx, bool& isOk)
    {
        int isnum;
        lua_Unsigned u = (lua_Unsigned)lua_tointegerx(pLua, varIdx, &isnum);
        isOk = isnum != 0;
        
        return static_cast<unsigned int>(u);
    }
    
    template<>
    bool LuaStack::getVariable<bool>(lua_State* pLua, int varIdx, bool& isOk)
    {
        isOk = lua_isboolean(pLua, varIdx) != 0;
        
        if(isOk)
            return lua_toboolean(pLua, varIdx) == 0 ? false : true;
        
        return false;
    }
    
    template<>
    double LuaStack::getVariable<double>(lua_State* pLua, int varIdx, bool& isOk)
    {
        int isnum;
        lua_Number d = lua_tonumberx(pLua, varIdx, &isnum);
        isOk = isnum != 0;
        
        return static_cast<double>(d);
    }
    
    template<>
    void* LuaStack::getVariable<void*>(lua_State* pLua, int varIdx, bool& isOk)
    {
        isOk = lua_isuserdata(pLua, varIdx) != 0;
        
        return lua_touserdata(pLua, varIdx);
    }
    
    //getVariable (no checking)
    
    template<>
    std::string LuaStack::getVariable<std::string>(lua_State* pLua, int varIdx)
    {
        const char* str = lua_tostring(pLua, varIdx);
        return std::string(str);
    }
    
    template<>
    int LuaStack::getVariable<int>(lua_State* pLua, int varIdx)
    {
        return static_cast<int>(lua_tointeger(pLua, varIdx));
    }
    
    template<>
    unsigned int LuaStack::getVariable<unsigned int>(lua_State* pLua, int varIdx)
    {
        return static_cast<unsigned int>(lua_tointeger(pLua, varIdx));
    }
    
    template<>
    bool LuaStack::getVariable<bool>(lua_State* pLua, int varIdx)
    {
        return lua_toboolean(pLua, varIdx) == 0 ? false : true;
    }
    
    template<>
    double LuaStack::getVariable<double>(lua_State* pLua, int varIdx)
    {
        lua_Number d = lua_tonumber(pLua, varIdx);
        return static_cast<double>(d);
    }
    
    template<>
    void* LuaStack::getVariable<void*>(lua_State* pLua, int varIdx)
    {
        return lua_touserdata(pLua, varIdx);
    }
    
    //pushVariable
    
    template<>
    void LuaStack::pushVariable<const char*>(lua_State* pLua, const char* data)
    {
        lua_pushstring(pLua, data);
    }
    
    template<>
    void LuaStack::pushVariable<std::string>(lua_State* pLua, std::string data)
    {
        lua_pushstring( pLua, data.c_str() );
    }
    
    template<>
    void LuaStack::pushVariable<const wchar_t*>(lua_State* pLua, const wchar_t* data)
    {
        lua_pushlstring( pLua, reinterpret_cast<const char*>(data), (wcslen(data)+1)*sizeof(wchar_t) );
    }
    
    template<>
    void LuaStack::pushVariable<std::wstring>(lua_State* pLua, std::wstring data)
    {
        lua_pushlstring( pLua, reinterpret_cast<const char*>(data.c_str() ), (data.size()+1)*sizeof(wchar_t) );
    }
    
    template<>
    void LuaStack::pushVariable<int>(lua_State* pLua, int data)
    {
        lua_pushinteger(pLua,	static_cast<lua_Integer>(data) );
    }
    
    template<>
    void LuaStack::pushVariable<unsigned int>(lua_State* pLua, unsigned int data)
    { 
        lua_pushinteger(pLua, static_cast<lua_Integer>(data) );
    }
    
    template<> 
    void LuaStack::pushVariable<bool>(lua_State* pLua, bool data)
    { 
        lua_pushboolean(pLua, static_cast<int>(data) );
    }
    
    template<> 
    void LuaStack::pushVariable<float>(lua_State* pLua, float data)
    { 
        lua_pushnumber(pLua, static_cast<lua_Number>(data) );
    }
    
    template<> 
    void LuaStack::pushVariable<double>(lua_State* pLua, double data)
    { 
        lua_pushnumber(pLua, static_cast<lua_Number>(data) );
    }
    
    template<>
    void LuaStack::pushVariable<void*>(lua_State* pLua, void* data)
    {
        lua_pushlightuserdata(pLua, data);
    }
}

#endif //LUALINK_DEFINE
