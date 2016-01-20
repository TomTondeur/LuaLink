//
//  main.cpp
//  LuaLinkTest
//
//  Created by Tom Tondeur on 06/04/2015.
//  Copyright (c) 2015 Tom Tondeur. All rights reserved.
//

#include <iostream>
#include <list>
#define LUALINK_DEFINE
#include "./../../LuaLink"

using namespace std;
using namespace LuaLink;

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

LUAFUNCTION(printMsg, "trace");
