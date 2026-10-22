#include "pch.h"
#include "Engine/LevelStreaming.h"
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
        .Get();

    return IsLevelVisible(this);
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
