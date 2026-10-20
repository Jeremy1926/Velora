#include "pch.h"
#include "HAL/IConsoleManager.h"
#include "Scanner.h"

IConsoleManager*& IConsoleManager::Singleton = *(IConsoleManager**)Scanner::String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms")).Start().Find("mov rcx, [0x*]; test rcx, rcx").Target(TEXT("IConsoleManager::Singleton"));

void IConsoleManager::SetupSingleton()
{
    static auto SetupSingleton = Anchor::Function<void (*)()>(TEXT("IConsoleManager::SetupSingleton"))
        .String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms"))
        .Start()
        .Find("mov rcx, [0x*]; test rcx, rcx")
        .Call()
        .Get();

    SetupSingleton();
}

IConsoleVariable* IConsoleManager::FindConsoleVariable(const TCHAR* Name) const
{
    static auto FindConsoleVariable = Anchor::Slot(TEXT("IConsoleManager::FindConsoleVariable"))
        .String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms"))
        .Start()
        .Find("mov rcx, [0x*]; test rcx, rcx")
        .Find("call [*+0x*]")
        .Get();

    return Scanner::Virtual<IConsoleVariable* (*)(const IConsoleManager*, const TCHAR*)>(this, FindConsoleVariable)(this, Name);
}

TConsoleVariableData<int32>* IConsoleObject::AsVariableInt()
{
    static auto AsVariableInt = Anchor::Slot(TEXT("IConsoleObject::AsVariableInt"))
        .String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms"))
        .Start()
        .Find("mov rcx, [0x*]; test rcx, rcx")
        .Find("call [*+0x*]", 2)
        .Get();

    return Scanner::Virtual<TConsoleVariableData<int32>* (*)(IConsoleObject*)>(this, AsVariableInt)(this);
}

void IConsoleVariable::Set(const TCHAR* InValue, EConsoleVariableFlags SetBy)
{
    static auto Set = Anchor::Slot(TEXT("IConsoleVariable::Set"))
        .String(TEXT("PLAYLIST: Garbage Collection is being overridden by Playlist: %s, and is setting it to %f"))
        .Last("call [rax+0x*]")
        .Get();

    Scanner::Virtual<void (*)(IConsoleVariable*, const TCHAR*, EConsoleVariableFlags)>(this, Set)(this, InValue, SetBy);
}

int32 IConsoleVariable::GetInt() const
{
    static auto GetInt = Anchor::Slot(TEXT("IConsoleVariable::GetInt"))
        .String(TEXT("sg.ShadowQuality"), "lea rdx, [0x*]; call [*+0x*]; mov rcx, rax; mov rdx, [rax]; call [*+0x*]")
        .Find("call [*+0x*]", 2)
        .Or()
        .String(TEXT("sg.ShadowQuality"), "lea rdx, [0x*]; mov r8b, 0x1; call [*+0x*]; mov rcx, rax; mov rdx, [rax]; call [*+0x*]")
        .Find("call [*+0x*]", 2)
        .Or()
        .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; mov r8b, 0x1; call [*+0x*]; mov rcx, rax; mov rdx, [rax]; call [*+0x*]")
        .Find("call [*+0x*]", 2)
        .Get();

    return Scanner::Virtual<int32 (*)(const IConsoleVariable*)>(this, GetInt)(this);
}
