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

namespace LuaLink {
    template<typename T>
    struct WeakLinkedList {
        struct node {
            T car;
            node* cdr;
            
            node(T _car, node* _cdr) : car(_car), cdr(_cdr) {}
        };
        
        node* m_begin;
    };
    
    struct LuaAutoClass {
        typedef void(*fn_register_statics_t)(void);
        typedef void(*fn_register_members_t)(void*);
        typedef void(*fn_register_class_t)(const std::string&, bool);
        
        template<typename T>
        LuaAutoClass(const char* name,
                     fn_register_statics_t fn_statics_and_methods,
                     void(*fn_vars)(T*),
                     fn_register_class_t fn_reg_class,
                     bool is_inheritance_allowed) :
        m_name(name),
        m_fn_reg_statics_and_methods(fn_statics_and_methods),
        m_fn_reg_vars(reinterpret_cast<fn_register_members_t>(fn_vars)),
        m_fn_reg_class(fn_reg_class),
        m_is_inheritance_allowed(is_inheritance_allowed)
        {
            
        }
        
        static WeakLinkedList<LuaAutoClass>::node* AddNode(WeakLinkedList<LuaAutoClass>::node* node) {
            auto result = AutoClassList().m_begin;
            AutoClassList().m_begin = node;
            return result;
        }
        
        static void RegisterAll() {
            for(auto elt = AutoClassList().m_begin; elt != nullptr; elt = elt->cdr)
                (*elt->car.m_fn_reg_class)(elt->car.m_name, elt->car.m_is_inheritance_allowed);
        }
        
    private:
        
        static WeakLinkedList<LuaAutoClass>& AutoClassList() {
            static WeakLinkedList<LuaAutoClass> s_AutoClassList;
            return s_AutoClassList;
        }
        
        const char* m_name;
        fn_register_statics_t m_fn_reg_statics_and_methods;
        fn_register_members_t m_fn_reg_vars;
        fn_register_class_t m_fn_reg_class;
        bool m_is_inheritance_allowed;
    };
    
    struct LuaAutoFunction {
        template<typename _RetType, typename... _ArgTypes>
        LuaAutoFunction(_RetType(*fn)(_ArgTypes...), const char* name) :
        m_fn(reinterpret_cast<void(*)(void)>(fn)),
        m_name(name),
        Register(RegisterImpl<_RetType, _ArgTypes...>)
        {
            
        }
        
        static WeakLinkedList<LuaAutoFunction>::node* AddNode(WeakLinkedList<LuaAutoFunction>::node* node) {
            auto result = AutoFunctionList().m_begin;
            AutoFunctionList().m_begin = node;
            return result;
        }
        
        static void RegisterAll() {
            for(auto elt = AutoFunctionList().m_begin; elt != nullptr; elt = elt->cdr)
                (*elt->car.Register)(elt->car);
        }
        
    private:
        void(*Register)(LuaAutoFunction&);
        void(*m_fn)(void);
        const char* m_name;
        
        static WeakLinkedList<LuaAutoFunction>& AutoFunctionList() {
            static WeakLinkedList<LuaAutoFunction> s_AutoFunctionList;
            return s_AutoFunctionList;
        }
        
        template<typename _RetType, typename... _ArgTypes>
        static void RegisterImpl(LuaAutoFunction& self) {
            LuaFunction::Register(reinterpret_cast<_RetType(*)(_ArgTypes...)>(self.m_fn), self.m_name);
        }
    };
}

#define GET_MACRO_2(_01, _02, NAME, ...) NAME
#define GET_MACRO_3(_01, _02, _03, NAME, ...) NAME

//Lua classes
#define LUACLASS_DECLARATION(X) \
static void RegisterStaticsAndMethods(void); \
static void RegisterVariables(X*); \
typedef X type

#define LUACLASS(...) GET_MACRO_3(__VA_ARGS__, LUACLASS_3, LUACLASS_2, LUACLASS_1)(__VA_ARGS__)
#define LUACLASS_1(CLASS) LUACLASS_2(CLASS,#CLASS)
#define LUACLASS_2(CLASS,NAME) LUACLASS_3(CLASS,NAME,true)
#define LUACLASS_3(CLASS,NAME,IS_INHERITANCE_ALLOWED) \
WeakLinkedList<LuaLink::LuaAutoClass>::node CLASS##_LuaClass_WLLN( \
LuaAutoClass(NAME, \
CLASS::RegisterStaticsAndMethods, \
CLASS::RegisterVariables, \
LuaClass<CLASS>::Register, \
IS_INHERITANCE_ALLOWED), \
LuaAutoClass::AddNode(&CLASS##_LuaClass_WLLN));

//Lua nonstatic members
#define LUAMEMBERS(CLASS) void CLASS::RegisterVariables(CLASS* self)
#define LUAMEMBER_1(X) LUAMEMBER_2(X, #X)
#define LUAMEMBER_2(X,NAME) LuaLink::LuaVariable::Register(self->X, NAME);
#define LUAMEMBER(...) GET_MACRO_2(__VA_ARGS__, LUAMEMBER_2, LUAMEMBER_1)(__VA_ARGS__)

//Lua statics and methods
#define LUASTATICMETHOD(...) GET_MACRO_2(__VA_ARGS__, LUASTATICMETHOD_2, LUASTATICMETHOD_1)(__VA_ARGS__)
#define LUASTATICMETHOD_1(X) LUASTATICMETHOD_2(X, #X)
#define LUASTATICMETHOD_2(X,NAME) LuaLink::LuaStaticMethod<type>::Register(X,NAME);

#define LUAMETHOD(...) GET_MACRO_2(__VA_ARGS__, LUAMETHOD_2, LUAMETHOD_1)(__VA_ARGS__)
#define LUAMETHOD_1(X) LUAMETHOD_2(X, #X)
#define LUAMETHOD_2(X,NAME) LuaLink::LuaMethod<type>::Register(&type::X,NAME);

#define LUASTATIC(...) GET_MACRO_2(__VA_ARGS__, LUASTATIC_2, LUASTATIC_1)(__VA_ARGS__)
#define LUASTATIC_1(X) LUASTATIC_2(X, #X)
#define LUASTATIC_2(X,NAME) LuaLink::LuaVariable::Register(X,NAME);

#define LUASTATICS(CLASS) void CLASS::RegisterStaticsAndMethods(void)

//Lua global functions
#define LUAFUNCTION(...) GET_MACRO_2(__VA_ARGS__, LUAFUNCTION_2, LUAFUNCTION_1)(__VA_ARGS__)
#define LUAFUNCTION_1(FN) LUAFUNCTION_2(FN,#FN)
#define LUAFUNCTION_2(FN,NAME) \
WeakLinkedList<LuaLink::LuaAutoFunction>::node FN##_LuaFunction_WLLN( \
LuaAutoFunction(FN,NAME), \
LuaAutoFunction::AddNode(&FN##_LuaFunction_WLLN));
