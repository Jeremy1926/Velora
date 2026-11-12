#include "pch.h"
#include "Engine/LevelStreaming.h"
#include "Engine/Level.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

ULevelStreaming::ECurrentState ULevelStreaming::GetCurrentState() const
{
    static auto CurrentState = Anchor::Offset(TEXT("ULevelStreaming::CurrentState"))
        .String(TEXT("Failed to load package '%s'"))
        .Find("mov byte ptr [*+0x*], 0x2")
        .Or()
        .String(TEXT("Failed to load package '%s'"), "lea rax, [0x*]; xor edx, edx; xor ecx, ecx")
        .Start()
        .CallSite()
        .Find("mov byte ptr [*+0x*], 0x2")
        .Or()
        .String(TEXT("Failed to load package '%s'"), "lea rax, [0x*]; xor edx, edx; xor ecx, ecx")
        .Start()
        .CallSite()
        .Find("test rcx, rcx; jz 0x*")
        .Find("jz 0x*")
        .Target()
        .Find("mov byte ptr [*+0x*], 0x2")
        .Or()
        .String(TEXT("Failed to load package '%s'"), "lea rax, [0x*]; mov r9d, 0x3")
        .Find("mov byte ptr [*+0x*], 0x2")
        .Or()
        .String(TEXT("Failed to load package '%s'"), "lea rax, [0x*]; mov r9d, 0x3")
        .Last("mov byte ptr [*+0x*], 0x2")
        .Or()
        .String(TEXT("Failed to load package '%s'"), "lea rax, [0x*]; xor edx, edx; xor ecx, ecx")
        .Start()
        .CallSite()
        .Find("mov dl, 0x2; mov rcx, r*; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov [*+0x*], dl")
        .Or()
        .String(TEXT("Failed to load package '%s'"), "lea rax, [0x*]; xor edx, edx; xor ecx, ecx")
        .Start()
        .CallSite()
        .Find("mov dl, 0x2; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("mov [*+0x*], dl")
        .Or()
        .Pattern("44 8A B9 ? ? ? ? 44 8A F2 4C 8B A1 ? ? ? ? 48 8B E9 88 91")
        .Find("mov [*+0x*], dl")
        .Or()
        .Pattern("48 89 6C 24 ? 48 8B E9 4C 89 64 24 ? 44 0F B6 E2 4C 89 6C 24 ? 44 0F B6 A9 ? ? ? ? 4C 89 74 24 ? 88 91")
        .Find("mov [*+0x*], dl")
        .Disp();

    return *(ECurrentState*)((uint8*)this + CurrentState);
}

ULevel* ULevelStreaming::GetLoadedLevel() const
{
    static auto LoadedLevel = GetClass()->FindPropertyByName(FName(TEXT("LoadedLevel")));

    return *LoadedLevel->ContainerPtrToValuePtr<ULevel*>(this);
}

bool ULevelStreaming::IsLevelLoaded() const
{
    return GetLoadedLevel() != nullptr;
}

bool ULevelStreaming::IsLevelVisible() const
{
    static auto IsLevelVisible = Anchor::Function<bool (*)(const ULevelStreaming*)>(TEXT("ULevelStreaming::IsLevelVisible"))
        .Native(TEXT("/Script/Engine.LevelStreaming:IsLevelVisible"))
        .Optional()
        .Get();

    if (IsLevelVisible)
        return IsLevelVisible(this);

    auto LoadedLevel = GetLoadedLevel();

    if (!LoadedLevel)
        return false;

    static auto bIsVisible = (UBoolProperty*)ULevel::StaticClass()->FindPropertyByName(FName(TEXT("bIsVisible")));

    return bIsVisible->GetPropertyValue_InContainer(LoadedLevel);
}

bool ULevelStreaming::ShouldBeLoaded() const
{
    static auto ShouldBeLoaded = Anchor::VirtualFunction<bool (*)(const ULevelStreaming*)>(TEXT("ULevelStreaming::ShouldBeLoaded"))
        .Exec(TEXT("/Script/Engine.LevelStreaming:ShouldBeLoaded"))
        .Find("call [rax+0x*]")
        .Get();

    return ShouldBeLoaded(this);
}

bool ULevelStreaming::ShouldBeVisible() const
{
    static auto ShouldBeVisible = Anchor::VirtualFunction<bool (*)(const ULevelStreaming*)>(TEXT("ULevelStreaming::ShouldBeLoaded"))
        .Exec(TEXT("/Script/Engine.LevelStreaming:ShouldBeLoaded"))
        .Find("call [rax+0x*]")
        .Get(sizeof(void*));

    return ShouldBeVisible(this);
}
