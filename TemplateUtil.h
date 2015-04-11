//
//  TemplateUtil.h
//  LuaLink
//
//  Created by Tom Tondeur on 09/04/2015.
//  Copyright (c) 2015 Tom Tondeur. All rights reserved.
//

#pragma once

#include "LuaStack.hpp"
#include <tuple>

namespace LuaLink {
    namespace detail {
        typedef int(*ArgErrorCbType)(lua_State*, int);
        
        //build_tuple_from_stack
        template<typename... T> struct build_tuple_from_lua_stack;
        
        template<typename T_Head>
        struct build_tuple_from_lua_stack<T_Head>
        {
            static std::tuple<T_Head> execute(lua_State* pLuaState, int argNum, bool& isOk, ArgErrorCbType onArgError, int& errRet)
            {
                auto var = LuaStack::getVariable<T_Head>(pLuaState, argNum, isOk);
                if(!isOk)
                    errRet = onArgError(pLuaState, argNum);
                
                return std::tuple<T_Head>(var);
            }
        };
        
        template<typename T_Head, typename... T_Tail>
        struct build_tuple_from_lua_stack<T_Head, T_Tail...>
        {
            static std::tuple<T_Head, T_Tail...> execute(lua_State* pLuaState, int argNum, bool& isOk, ArgErrorCbType onArgError, int& errRet)
            {
                auto tpl_head = build_tuple_from_lua_stack<T_Head>::execute(pLuaState, argNum, isOk, onArgError, errRet);
                auto tpl_tail = build_tuple_from_lua_stack<T_Tail...>::execute(pLuaState, argNum+1, isOk, onArgError, errRet);
                return std::tuple_cat(tpl_head, std::move(tpl_tail));
            }
        };
        
        //call
        template <typename F, typename Tuple, bool Done, int Total, int... N>
        struct call_impl
        {
            static auto call(F f, Tuple && t) -> decltype(call_impl<F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(f, std::forward<Tuple>(t)))
            {
                return call_impl<F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(f, std::forward<Tuple>(t));
            }
        };
        
        template <typename F, typename Tuple, int Total, int... N>
        struct call_impl<F, Tuple, true, Total, N...>
        {
            static auto call(F f, Tuple && t) -> decltype(f(std::get<N>(std::forward<Tuple>(t))...))
            {
                return f(std::get<N>(std::forward<Tuple>(t))...);
            }
        };
        
        template <typename F, typename Tuple>
        auto call(F f, Tuple && t) -> decltype(call_impl<F, Tuple, 0 == std::tuple_size<typename std::decay<Tuple>::type>::value, std::tuple_size<typename std::decay<Tuple>::type>::value>::call(f, std::forward<Tuple>(t)))
        {
            return call_impl<F, Tuple, 0 == std::tuple_size<typename std::decay<Tuple>::type>::value, std::tuple_size<typename std::decay<Tuple>::type>::value>::call(f, std::forward<Tuple>(t));
        }
        
        template <typename F, typename P, typename Tuple, bool Done, int Total, int... N>
        struct call_mem_impl
        {
            static auto call(F f, P p, Tuple && t) -> decltype(call_mem_impl<F, P, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(f, p, std::forward<Tuple>(t)))
            {
                return call_mem_impl<F, P, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(f, p, std::forward<Tuple>(t));
            }
        };
        
        template <typename F, typename P, typename Tuple, int Total, int... N>
        struct call_mem_impl<F, P, Tuple, true, Total, N...>
        {
            static auto call(F f, P p, Tuple && t) -> decltype((p->*f)(std::get<N>(std::forward<Tuple>(t))...))
            {
                return (p->*f)(std::get<N>(std::forward<Tuple>(t))...);
            }
        };
        
        template <typename F, typename P, typename Tuple>
        auto call_mem(F f, P p, Tuple && t) -> decltype(call_mem_impl<F, P, Tuple, 0 == std::tuple_size<typename std::decay<Tuple>::type>::value, std::tuple_size<typename std::decay<Tuple>::type>::value>::call(f, p, std::forward<Tuple>(t)))
        {
            return call_mem_impl<F, P, Tuple, 0 == std::tuple_size<typename std::decay<Tuple>::type>::value, std::tuple_size<typename std::decay<Tuple>::type>::value>::call(f, p, std::forward<Tuple>(t));
        }
        
        struct CStrCmp {
            bool operator()(const char* a, const char* b) const;
        };
        
#ifdef LUALINK_DEFINE
        bool CStrCmp::operator()(const char* a, const char* b) const
        {
            return strcmp(a, b) < 0;
        }
#endif
    }
}

