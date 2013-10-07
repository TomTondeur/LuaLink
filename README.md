LuaLink
=======

A library designed to seemlessly embed Lua in any C++ application. This library was designed to embed Lua with as little code as possible. 

How to use
----------

Use the LuaScript class to open up a .lua file. 
The LuaFunction class can be used to register global functions. 
The template class LuaClass will assume the template argument (the class to expose to Lua), contains the static member function `void RegisterStaticsAndMethods(void)` and nonstatic member function `void RegisterVariables(void)`.

In `RegisterStaticsAndMethods`, you can use the LuaVariable, LuaStaticMethod and LuaMethod classes to register static variables, static methods and nonstatic methods respectively.

In `RegisterVariables`, you can use the LuaVariable class to register nonstatic member variables.

Constructors have to be implemented as static methods and registered with the name "new". In order to inherit from C++ classes in Lua, you can call the inherit() method that is automatically generated for every class (this behaviour can be switched off).

Here's a sample that puts all of this into practise:

```
//main.cpp
#include "stdafx.h"
#include <LuaLink>
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

		luaScript.CallLuaMethod<void>("EventHandler", "TriggerEvent", "GameStart");
		luaScript.CallLuaFunction<int>("Run");
	}
	catch(std::exception& e){
		cout << endl << e.what() << endl;
	}
	
	system("pause");
	
	return 0;
}
```

```
--demo.lua
print("\nStarting demo.lua")

--Demo event handler

EventHandler = {}
EventHandler.CreateEvent = {}
function EventHandler.TriggerEvent(str_event)
	if EventHandler[str_event] ~= nil then
		for i, v in ipairs(EventHandler[str_event]) do
			v()
		end
	else
		trace("No event handlers for \"" .. str_event .. "\"")
	end
end

function EventHandler.Register(events)
	for eventName, fn in pairs(events) do
		if EventHandler[eventName] == nil then
			EventHandler[eventName] = {}
		end
		table.insert(EventHandler[eventName], fn)
	end	
end

--Add event listener

MyEventListener = {}
function MyEventListener.GameStart()
	print("Game started")
end

EventHandler.Register(MyEventListener)

--Inherit from registered C++ class

BasicAccount = Account.inherit()

--Add fields to C++ class (we could add this to BasicAccount too, just showing we can extend C++ classes
function Account:show()
	print("Balance of this account is " .. self.Balance:get())
end

--Constructor for our new class
function BasicAccount:new(val)
	newObj = Account.new(val)	
	self.__index = self;
	return setmetatable(newObj, self)
end

--Runs a small demo
function Run()
	b = BasicAccount:new(200)
	b:Deposit(30)
	b:show()
	print(b)
end
```

Have fun exploring this library and I hope it will prove useful in your projects.
