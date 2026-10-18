#include "pch.h"
#include "HAL/IConsoleManager.h"
#include "Scanner.h"

IConsoleManager*& IConsoleManager::Singleton = *(IConsoleManager**)Scanner::String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")).Start().Find("mov rcx, [0x*]; test rcx, rcx").Target();

void IConsoleManager::SetupSingleton()
{
    static auto SetupSingleton = (void (*)())Scanner::String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")).Start().Find("mov rcx, [0x*]; test rcx, rcx").Call().Get();

    SetupSingleton();
}

IConsoleVariable* IConsoleManager::FindConsoleVariable(const TCHAR* Name) const
{
    static auto FindConsoleVariable = Scanner::String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")).Start().Find("mov rcx, [0x*]; test rcx, rcx").Find("call [*+0x*]").Disp();

    auto VTable = *(uint8_t**)this;

    return (*(IConsoleVariable* (**)(const IConsoleManager*, const TCHAR*))(VTable + FindConsoleVariable))(this, Name);
}

TConsoleVariableData<int32>* IConsoleObject::AsVariableInt()
{
    static auto AsVariableInt = Scanner::String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")).Start().Find("mov rcx, [0x*]; test rcx, rcx").Find("call [*+0x*]", 2).Disp();

    auto VTable = *(uint8_t**)this;

    return (*(TConsoleVariableData<int32>* (**)(IConsoleObject*))(VTable + AsVariableInt))(this);
}

int32 IConsoleVariable::GetInt() const
{
    static auto GetInt = Scanner::String(TEXT("athena.Vehicles")).Find("call [*+0x*]", 2).Disp();

    auto VTable = *(uint8_t**)this;

    return (*(int32 (**)(const IConsoleVariable*))(VTable + GetInt))(this);
}
