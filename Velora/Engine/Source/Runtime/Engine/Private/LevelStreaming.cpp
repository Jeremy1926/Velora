#include "pch.h"
#include "Engine/LevelStreaming.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

ULevelStreaming::ECurrentState ULevelStreaming::GetCurrentState() const
{
    static auto CurrentState = Scanner::String(TEXT("Failed to load package '%s'")).Find("mov byte ptr [*+0x*], 0x2").Disp(TEXT("ULevelStreaming::CurrentState"));

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
    static auto ShouldBeLoaded = Anchor::Slot(TEXT("ULevelStreaming::ShouldBeLoaded"))
        .Exec(TEXT("/Script/Engine.LevelStreaming:ShouldBeLoaded"))
        .Find("call [rax+0x*]")
        .Get();

    return Scanner::Virtual<bool (*)(const ULevelStreaming*)>(this, ShouldBeLoaded)(this);
}

bool ULevelStreaming::ShouldBeVisible() const
{
    static auto ShouldBeLoaded = Anchor::Slot(TEXT("ULevelStreaming::ShouldBeLoaded"))
        .Exec(TEXT("/Script/Engine.LevelStreaming:ShouldBeLoaded"))
        .Find("call [rax+0x*]")
        .Get();

    return Scanner::Virtual<bool (*)(const ULevelStreaming*)>(this, ShouldBeLoaded, sizeof(void*))(this);
}
