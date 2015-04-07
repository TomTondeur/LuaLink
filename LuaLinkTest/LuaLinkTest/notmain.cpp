//
//  notmain.cpp
//  LuaLinkTest
//
//  Created by Tom Tondeur on 07/04/2015.
//  Copyright (c) 2015 Tom Tondeur. All rights reserved.
//

#include <stdio.h>
//#define LUALINK_DEFINE
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