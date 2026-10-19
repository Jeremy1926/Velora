#include "pch.h"
#include "HAL/IConsoleManager.h"
#include "Scanner.h"

IConsoleManager*& IConsoleManager::Singleton = *(IConsoleManager**)Scanner::String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms")).Start().Find("mov rcx, [0x*]; test rcx, rcx").Target(TEXT("IConsoleManager::Singleton"));

void IConsoleManager::SetupSingleton()
{
    static auto SetupSingleton = Scanner::Function<void (*)()>(Scanner::String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms")).Start().Find("mov rcx, [0x*]; test rcx, rcx").Call().Get(TEXT("IConsoleManager::SetupSingleton")));

    SetupSingleton();
}

IConsoleVariable* IConsoleManager::FindConsoleVariable(const TCHAR* Name) const
{
    static auto FindConsoleVariable = Scanner::String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms")).Start().Find("mov rcx, [0x*]; test rcx, rcx").Find("call [*+0x*]").Slot(TEXT("IConsoleManager::FindConsoleVariable"));

    return Scanner::Virtual<IConsoleVariable* (*)(const IConsoleManager*, const TCHAR*)>(this, FindConsoleVariable)(this, Name);
}

TConsoleVariableData<int32>* IConsoleObject::AsVariableInt()
{
    static auto AsVariableInt = Scanner::String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms")).Start().Find("mov rcx, [0x*]; test rcx, rcx").Find("call [*+0x*]", 2).Slot(TEXT("IConsoleObject::AsVariableInt"));

    return Scanner::Virtual<TConsoleVariableData<int32>* (*)(IConsoleObject*)>(this, AsVariableInt)(this);
}

void IConsoleVariable::Set(const TCHAR* InValue, EConsoleVariableFlags SetBy)
{
    static auto Set = Scanner::String(TEXT("PLAYLIST: Garbage Collection is being overridden by Playlist: %s, and is setting it to %f")).Last("call [rax+0x*]").Slot(TEXT("IConsoleVariable::Set"));

    Scanner::Virtual<void (*)(IConsoleVariable*, const TCHAR*, EConsoleVariableFlags)>(this, Set)(this, InValue, SetBy);
}

int32 IConsoleVariable::GetInt() const
{
    static auto GetInt = Scanner::String(TEXT("sg.ShadowQuality"), "lea rdx, [0x*]; call [*+0x*]; mov rcx, rax; mov rdx, [rax]; call [*+0x*]").Find("call [*+0x*]", 2).Slot(TEXT("IConsoleVariable::GetInt"));

    return Scanner::Virtual<int32 (*)(const IConsoleVariable*)>(this, GetInt)(this);
}
