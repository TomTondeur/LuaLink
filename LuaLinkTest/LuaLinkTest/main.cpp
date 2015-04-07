//
//  main.cpp
//  LuaLinkTest
//
//  Created by Tom Tondeur on 06/04/2015.
//  Copyright (c) 2015 Tom Tondeur. All rights reserved.
//

#include <iostream>
#include <list>
#include "LuaLink"

using namespace std;
using namespace LuaLink;

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
        void(*Register)(LuaAutoFunction&); //To be implemented by templated classes
        void(*m_fn)(void); //Generic variable to hold function to register, templated classes should cast to correct type
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

#define LUACLASS(...) GET_MACRO_3(__VA_ARGS__, LUACLASS_3, LUACLASS_2, LUACLASS_1)(__VA_ARGS__)

//Lua nonstatic members
#define LUAMEMBER_1(X) LUAMEMBER_2(X, #X)
#define LUAMEMBER_2(X,NAME) LuaLink::LuaVariable::Register(self->X, NAME);
#define LUAMEMBER(...) GET_MACRO_2(__VA_ARGS__, LUAMEMBER_2, LUAMEMBER_1)(__VA_ARGS__)
#define LUAMEMBERS(CLASS) void CLASS::RegisterVariables(CLASS* self)

//Lua statics and methods
#define LUASTATICMETHOD_1(X) LUASTATICMETHOD_2(X, #X)
#define LUASTATICMETHOD_2(X,NAME) LuaLink::LuaStaticMethod<type>::Register(X,NAME);
#define LUASTATICMETHOD(...) GET_MACRO_2(__VA_ARGS__, LUASTATICMETHOD_2, LUASTATICMETHOD_1)(__VA_ARGS__)

#define LUAMETHOD_1(X) LUAMETHOD_2(X, #X)
#define LUAMETHOD_2(X,NAME) LuaLink::LuaMethod<type>::Register(&type::X,NAME);
#define LUAMETHOD(...) GET_MACRO_2(__VA_ARGS__, LUAMETHOD_2, LUAMETHOD_1)(__VA_ARGS__)

#define LUASTATIC_1(X) LUASTATIC_2(X, #X)
#define LUASTATIC_2(X,NAME) LuaLink::LuaVariable::Register(X,NAME);
#define LUASTATIC(...) GET_MACRO_2(__VA_ARGS__, LUASTATIC_2, LUASTATIC_1)(__VA_ARGS__)
#define LUASTATICS(CLASS) void CLASS::RegisterStaticsAndMethods(void)

//Lua global functions
#define LUAFUNCTION_1(FN) LUAFUNCTION_2(FN,#FN)
#define LUAFUNCTION_2(FN,NAME) \
WeakLinkedList<LuaLink::LuaAutoFunction>::node FN##_LuaFunction_WLLN( \
LuaAutoFunction(FN,NAME), \
LuaAutoFunction::AddNode(&FN##_LuaFunction_WLLN));

#define LUAFUNCTION(...) GET_MACRO_2(__VA_ARGS__, LUAFUNCTION_2, LUAFUNCTION_1)(__VA_ARGS__)

class Account {
    int m_Balance;
    
public:
    Account(int amount):m_Balance(amount){}
    ~Account() { printf("deleted Account (%p)\n", this); }
    
    void Deposit(int amount){ m_Balance += amount; }
    void Withdraw(int amount){ m_Balance -= amount; }
    
    //Passthrough function to expose constructor, note that we return void*
    static void* LuaNew(int amount){return static_cast<void*>(new Account(amount));}
    
    static int x;
    static int y;
    
    LUACLASS_DECLARATION(Account);
};

int Account::x = 0;
int Account::y = 0;

void printMsg(std::string str)
{
    printf("%s\n", str.c_str());
}

void InitEnvironment(void)
{
    LuaAutoFunction::RegisterAll();
    LuaAutoClass::RegisterAll();
}

int main()
{
    try{
        LuaScript luaScript("demo.lua");
        
        luaScript.Load(InitEnvironment, true, false); //Pass our initializer callback
        luaScript.Initialize();
        
        luaScript.CallMethod<void>("EventHandler", "TriggerEvent", "GameStart");
        luaScript.CallFunction<int>("Run");
    }
    catch(std::exception& e){
        cout << endl << e.what() << endl;
    }
    
    cout << "Press any key to continue...";
    flush(cout);
    system("read");
    
    return 0;
}

LUACLASS(Account);

LUASTATICS(Account) {
    //Nonstatic methods
    LUAMETHOD(Deposit);
    LUAMETHOD(Withdraw);
    
    //Static methods
    LUASTATICMETHOD(LuaNew, "new");
    
    //Static variables
    LUASTATIC(x);
    LUASTATIC(y);
}

//Nonstatic variables
LUAMEMBERS(Account) {
    LUAMEMBER(m_Balance, "Balance");
}

LUAFUNCTION(printMsg, "trace");
