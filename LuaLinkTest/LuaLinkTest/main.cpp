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

int main()
{
    try{
        LuaScript luaScript("demo.lua");
        
        luaScript.Load(nullptr, true, false); //Pass our initializer callback
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
