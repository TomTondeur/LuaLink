//
//  main.cpp
//  LuaLinkTest
//
//  Created by Tom Tondeur on 06/04/2015.
//  Copyright (c) 2015 Tom Tondeur. All rights reserved.
//

#include <iostream>
#include "LuaLink"

using namespace std;
using namespace LuaLink;

class Account {
    int m_Balance;
    
public:
    Account(int amount):m_Balance(amount){}
    ~Account() { printf("deleted Account (%p)\n", this); }
    
    void Deposit(int amount){ m_Balance += amount; }
    void Withdraw(int amount){ m_Balance -= amount; }
    
    //Passthrough function to expose constructor, note that we return void*
    static void* LuaNew(int amount){return static_cast<void*>(new Account(amount));}
    
    //These functions are automatically called when registering this class in Lua
    //
    static void RegisterStaticsAndMethods(void)
    {
        //Nonstatic methods
        LuaMethod<Account>::Register(&Account::Deposit, "Deposit");
        LuaMethod<Account>::Register(&Account::Withdraw, "Withdraw");
        
        //Static methods
        LuaStaticMethod<Account>::Register(LuaNew, "new");
        
        //Static variables (none)
    }
    
    void RegisterVariables(void)
    {
        //Nonstatic variables
        LuaVariable::Register(m_Balance, "Balance");
    }
};

void printMsg(std::string str)
{
    printf("%s\n", str.c_str());
}

//Register classes, global functions and global variables here
void InitEnvironment(void)
{
    LuaFunction::Register(printMsg, "trace");
    LuaClass<Account>::Register("Account");
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
