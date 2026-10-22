#include "pch.h"
#include "HAL/IConsoleManager.h"
#include "Scanner.h"

IConsoleManager*& IConsoleManager::Singleton = *Anchor::Data<IConsoleManager**>(TEXT("IConsoleManager::Singleton"))
    .String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms"))
    .Start()
    .Find("mov rcx, [0x*]; test rcx, rcx")
    .Target()
    .Get();

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
    static auto FindConsoleVariable = Anchor::VirtualFunction<IConsoleVariable* (*)(const IConsoleManager*, const TCHAR*)>(TEXT("IConsoleManager::FindConsoleVariable"))
        .String(TEXT("vm.OptimizeVMByteCode"), "lea rdx, [0x*]; mov r8b, 0x1")
        .Find("call [rax+0x*]")
        .Or()
        .String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms"))
        .Start()
        .Find("mov rcx, [0x*]; test rcx, rcx")
        .Find("call [*+0x*]")
        .Get();

    return FindConsoleVariable(this, Name);
}

TConsoleVariableData<int32>* IConsoleObject::AsVariableInt()
{
    static auto AsVariableInt = Anchor::VirtualFunction<TConsoleVariableData<int32>* (*)(IConsoleObject*)>(TEXT("IConsoleObject::AsVariableInt"))
        .String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms"))
        .Start()
        .Find("mov rcx, [0x*]; test rcx, rcx")
        .Find("call [*+0x*]", 2)
        .Get();

    return AsVariableInt(this);
}

void IConsoleVariable::Set(const TCHAR* InValue, EConsoleVariableFlags SetBy)
{
    static auto Set = Anchor::VirtualFunction<void (*)(IConsoleVariable*, const TCHAR*, EConsoleVariableFlags)>(TEXT("IConsoleVariable::Set"))
        .String(TEXT("PLAYLIST: Garbage Collection is being overridden by Playlist: %s, and is setting it to %f"))
        .Is("call 0x*")
        .Last("mov r8d, 0x8000000")
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]")
        .Or()
        .String(TEXT("PLAYLIST: Garbage Collection is being overridden by Playlist: %s, and is setting it to %f"))
        .Last("call [rax+0x*]")
        .Get();

    Set(this, InValue, SetBy);
}

int32 IConsoleVariable::GetInt() const
{
    static auto GetInt = Anchor::VirtualFunction<int32 (*)(const IConsoleVariable*)>(TEXT("IConsoleVariable::GetInt"))
        .String(TEXT("sg.ShadowQuality"), "lea rdx, [0x*]; call [*+0x*]; mov rcx, rax; mov rdx, [rax]; call [*+0x*]")
        .Find("call [*+0x*]", 2)
        .Or()
        .String(TEXT("sg.ShadowQuality"), "lea rdx, [0x*]; mov r8b, 0x1; call [*+0x*]; mov rcx, rax; mov rdx, [rax]; call [*+0x*]")
        .Find("call [*+0x*]", 2)
        .Or()
        .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; mov r8b, 0x1; call [*+0x*]; mov rcx, rax; mov rdx, [rax]; call [*+0x*]")
        .Find("call [*+0x*]", 2)
        .Or()
        .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; mov r8b, *; call [*+0x*]; mov rcx, rax; mov rdx, [rax]; call [*+0x*]")
        .Find("call [*+0x*]", 2)
        .Get();

    return GetInt(this);
}
