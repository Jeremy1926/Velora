#include "pch.h"
#include "HAL/IConsoleManager.h"
#include "Containers/UnrealString.h"
#include "Scanner.h"

IConsoleManager*& IConsoleManager::Singleton = *Anchor::Data<IConsoleManager**>(TEXT("IConsoleManager::Singleton"))
    .String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms"))
    .Start()
    .Find("mov rcx, [0x*]; test rcx, rcx")
    .Target()
    .Or()
    .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; mov r8b, *; call [*+0x*]; mov rcx, rax; mov rdx, [rax]; call [*+0x*]")
    .Last("mov rcx, [0x*]; test rcx, rcx")
    .Target()
    .Or()
    .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; mov r8b, *; call [*+0x*]; mov rcx, rax; mov rdx, [rax]; call [*+0x*]")
    .Last("call 0x*; mov rcx, [0x*]; mov rax, [rcx]")
    .Find("mov rcx, [0x*]")
    .Target()
    .Or()
    .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; mov r8b, *; call [*+0x*]; mov rcx, rax; mov rdx, [rax]; call [*+0x*]")
    .Last("call 0x*; mov rcx, [0x*]; jmp 0x*")
    .Find("mov rcx, [0x*]")
    .Target()
    .Or()
    .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; mov r8b, *; call [*+0x*]; mov rcx, rax; mov rdx, [rax]; call [*+0x*]")
    .Last("mov rcx, [0x*]; mov rax, [rcx]")
    .Target()
    .Or()
    .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; mov r8b, *; call [*+0x*]; mov rdx, [rax]; mov rcx, rax; call [*+0x*]")
    .Last("mov rcx, [0x*]; test rcx, rcx")
    .Target()
    .Or()
    .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; *; *; call [rax+0x*]; *; *; call [*+0x*]")
    .Last("mov rcx, [0x*]; test rcx, rcx")
    .Target()
    .Get();

void IConsoleManager::SetupSingleton()
{
    static auto SetupSingleton = Anchor::Function<void (*)()>(TEXT("IConsoleManager::SetupSingleton"))
        .String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms"))
        .Start()
        .Find("mov rcx, [0x*]; test rcx, rcx")
        .Call()
        .Or()
        .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; mov r8b, *; call [*+0x*]; mov rcx, rax; mov rdx, [rax]; call [*+0x*]")
        .Last("mov rcx, [0x*]; test rcx, rcx")
        .Call()
        .Or()
        .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; mov r8b, *; call [*+0x*]; mov rcx, rax; mov rdx, [rax]; call [*+0x*]")
        .Last("call 0x*; mov rcx, [0x*]; mov rax, [rcx]")
        .Call()
        .Or()
        .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; mov r8b, *; call [*+0x*]; mov rcx, rax; mov rdx, [rax]; call [*+0x*]")
        .Last("call 0x*; mov rcx, [0x*]; jmp 0x*")
        .Call()
        .Or()
        .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; mov r8b, *; call [*+0x*]; mov rdx, [rax]; mov rcx, rax; call [*+0x*]")
        .Last("mov rcx, [0x*]; test rcx, rcx; jz 0x*")
        .Find("jz 0x*")
        .Target()
        .Is("call 0x*")
        .Target()
        .Or()
        .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; *; *; call [rax+0x*]; *; *; call [*+0x*]")
        .Last("mov rcx, [0x*]; test rcx, rcx")
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
        .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; mov r8b, *; call [*+0x*]; mov rcx, rax; mov rdx, [rax]; call [*+0x*]")
        .Find("call [*+0x*]")
        .Or()
        .String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms"))
        .Start()
        .Find("mov rcx, [0x*]; test rcx, rcx")
        .Find("call [*+0x*]")
        .Or()
        .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; mov r8b, *; call [*+0x*]; mov rdx, [rax]; mov rcx, rax; call [*+0x*]")
        .Find("call [*+0x*]")
        .Or()
        .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; *; *; call [rax+0x*]; *; *; call [*+0x*]")
        .Find("call [*+0x*]")
        .Get();

    return FindConsoleVariable(this, Name);
}

TConsoleVariableData<int32>* IConsoleObject::AsVariableInt()
{
    static auto AsVariableInt = Anchor::VirtualFunction<TConsoleVariableData<int32>* (*)(IConsoleObject*)>(TEXT("IConsoleObject::AsVariableInt"))
        .String(TEXT("r.MobileHDR"), "lea rdx, [0x*]; call 0x*; lea rcx, [0x*]; mov [0x*], rax")
        .Find("call 0x*")
        .Target()
        .Find("call [*+0x*]", 2)
        .Or()
        .String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms"))
        .Start()
        .Find("mov rcx, [0x*]; test rcx, rcx")
        .Find("call [*+0x*]", 2)
        .Or()
        .String(TEXT("r.MobileHDR"), "lea rdx, [0x*]; mov r8b, 0x1; call [*+0x*]; test rax, rax; jz *; mov rdx, [rax]; mov rcx, rax; call [*+0x*]")
        .Find("call [*+0x*]", 2)
        .Or()
        .String(TEXT("r.MobileHDR"), "lea rdx, [0x*]; mov r8b, 0x1; call [rax+0x*]")
        .Find("call [*+0x*]", 2)
        .Or()
        .String(TEXT("r.MobileHDR"), "lea rdx, [0x*]; mov r8b, *; call [*+0x*]; test rax, rax; jz *; mov rdx, [rax]; mov rcx, rax; call [*+0x*]")
        .Find("call [*+0x*]", 2)
        .Or()
        .String(TEXT("r.MobileHDR"), "lea rdx, [0x*]; call [*+0x*]; test rax, rax; jz *; mov rdx, [rax]; mov rcx, rax; call [*+0x*]")
        .Find("call [*+0x*]", 2)
        .Or()
        .String(TEXT("r.MobileHDR"), "lea rdx, [0x*]; call [*+0x*]; test rax, rax; jz *; mov rcx, rax; mov rdx, [rax]; call [*+0x*]")
        .Find("call [*+0x*]", 2)
        .Get();

    return AsVariableInt(this);
}

void IConsoleVariable::Set(const TCHAR* InValue, EConsoleVariableFlags SetBy)
{
    static auto Set = Anchor::VirtualFunction<void (*)(IConsoleVariable*, const TCHAR*, EConsoleVariableFlags, int32)>(TEXT("IConsoleVariable::Set"))
        .String(TEXT("PLAYLIST: Garbage Collection is being overridden by Playlist: %s, and is setting it to %f"))
        .Is("call 0x*")
        .Last("mov r8d, 0x8000000")
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]")
        .Or()
        .String(TEXT("PLAYLIST: Garbage Collection is being overridden by Playlist: %s, and is setting it to %f"))
        .Is("call 0x*")
        .Last("mov r8d, 0x9000000")
        .Find("call 0x*")
        .Target()
        .Find("mov *, [rax+0x*]")
        .Or()
        .String(TEXT("PLAYLIST: Garbage Collection is being overridden by Playlist: %s, and is setting it to %f"))
        .Is("call 0x*")
        .Last("mov r8d, 0x8000000")
        .Find("call 0x*")
        .Target()
        .Find("mov *, [rax+0x*]")
        .Or()
        .String(TEXT("PLAYLIST: Garbage Collection is being overridden by Playlist: %s, and is setting it to %f"))
        .Last("mov r8d, 0x9000000")
        .Find("call 0x*")
        .Target()
        .Find("mov *, [rax+0x*]")
        .Or()
        .String(TEXT("PLAYLIST: Garbage Collection is being overridden by Playlist: %s, and is setting it to %f"))
        .Last("mov r8d, 0xc000000")
        .Find("call 0x*")
        .Target()
        .Find("mov *, [rax+0x*]")
        .Or()
        .String(TEXT("PLAYLIST: Garbage Collection is being overridden by Playlist: %s, and is setting it to %f"))
        .Last("mov r8d, 0xa000000")
        .Find("call 0x*")
        .Target()
        .Find("mov *, [rax+0x*]")
        .Or()
        .String(TEXT("PLAYLIST: Garbage Collection is being overridden by Playlist: %s, and is setting it to %f"))
        .Last("mov r8d, 0xb000000")
        .Find("call 0x*")
        .Target()
        .Find("mov *, [rax+0x*]")
        .Or()
        .String(TEXT("PLAYLIST: Garbage Collection is being overridden by Playlist: %s, and is setting it to %f"))
        .Last("mov r8d, 0xd000000")
        .Find("call 0x*")
        .Target()
        .Find("mov *, [rax+0x*]")
        .Or()
        .String(TEXT("PLAYLIST: Garbage Collection is being overridden by Playlist: %s, and is setting it to %f"))
        .Last("call [rax+0x*]")
        .Get();

    Set(this, InValue, SetBy, 0);
}

void IConsoleVariable::Set(int32 InValue, EConsoleVariableFlags SetBy)
{
    Set(*FString::FromInt(InValue), SetBy);
}

void IConsoleVariable::SetOnChangedCallback(const FConsoleVariableDelegate& Callback)
{
    static auto SetOnChangedCallback = Anchor::VirtualFunction<void (*)(IConsoleVariable*, const FConsoleVariableDelegate&)>(TEXT("IConsoleVariable::SetOnChangedCallback"))
        .String(TEXT("Slate.EnableGlobalInvalidation"), "lea rdx, [0x*]; mov r8b, *; call [rax+0x*]; test rax, rax; jz 0x*; mov r8, [rax]")
        .Find("call [r8+0x*]")
        .Or()
        .String(TEXT("Slate.EnableGlobalInvalidation"), "lea rdx, [0x*]; mov r8b, *; call [rax+0x*]; mov r9, rax; test rax, rax; jz 0x*; mov rcx, [rax]; mov r8, [rcx+0x*]")
        .Find("mov r8, [rcx+0x*]")
        .Or()
        .String(TEXT("Slate.EnableGlobalInvalidation"), "lea rdx, [0x*]; mov r8b, *; call [rax+0x*]; *; test rax, rax; jz 0x*; mov r8, [rax]")
        .Find("call [r8+0x*]")
        .Or()
        .String(TEXT("Slate.EnableGlobalInvalidation"), "lea rdx, [0x*]; mov r12d, *; mov r8b, r12b; call [rax+0x*]; *; *; test rax, rax; jz 0x*; mov r8, [rax]")
        .Find("call [r8+0x*]")
        .Or()
        .String(TEXT("Slate.EnableGlobalInvalidation"), "lea rdx, [0x*]; mov r8b, *; call [rax+0x*]; test rax, rax; jz 0x*; xorps *, *; lea *, [rsp+0x*]; movaps [*], *; mov r8, [rax]")
        .Find("call [r8+0x*]")
        .Or()
        .String(TEXT("Slate.EnableGlobalInvalidation"), "lea rdx, [0x*]; call [rax+0x*]; test rax, rax; jz 0x*; mov r8, [rax]")
        .Find("call [r8+0x*]")
        .Get();

    SetOnChangedCallback(this, Callback);
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
        .Or()
        .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; call [*+0x*]; mov rdx, [rax]; mov rcx, rax; call [*+0x*]; test eax, eax")
        .Find("call [*+0x*]", 2)
        .Or()
        .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; mov r8b, *; call [*+0x*]; mov rdx, [rax]; mov rcx, rax; call [*+0x*]; test eax, eax")
        .Find("call [*+0x*]", 2)
        .Or()
        .String(TEXT("sg.ViewDistanceQuality"), "lea rdx, [0x*]; *; *; call [rax+0x*]; *; *; call [*+0x*]")
        .Find("call [*+0x*]", 2)
        .Get();

    return GetInt(this);
}

float IConsoleVariable::GetFloat() const
{
    static auto GetFloat = Anchor::VirtualFunction<float (*)(const IConsoleVariable*)>(TEXT("IConsoleVariable::GetFloat"))
        .String(TEXT("r.ViewDistanceScale"), "lea rdx, [0x*]; mov r8b, *; call [*+0x*]; mov rcx, rax; mov rbx, rax; mov rdx, [rax]; call [*+0x*]; movss *, *")
        .Find("call [*+0x*]", 2)
        .Or()
        .String(TEXT("r.ViewDistanceScale"), "lea rdx, [0x*]; mov r8b, *; call [*+0x*]; mov rdx, [rax]; mov rcx, rax; call [*+0x*]; maxss *, *")
        .Find("call [*+0x*]", 2)
        .Get();

    return GetFloat(this);
}
