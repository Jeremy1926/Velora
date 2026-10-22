#include "pch.h"
#include "GameFeaturesSubsystem.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UGameFeaturesSubsystem* UGameFeaturesSubsystem::Get()
{
    static auto Get = Anchor::Optional<UGameFeaturesSubsystem* (*)()>(TEXT("UGameFeaturesSubsystem::Get"))
        .String(TEXT(" - Adding preload object: '%s'"))
        .Caller()
        .Find("call 0x*; mov rcx, rax; lea rdx, [*]; call 0x*; mov rsi, [*]")
        .Call()
        .Get();

    return Get ? Get() : nullptr;
}

void UGameFeaturesSubsystem::GetGameFeatureDataForActivePlugins(TArray<const UGameFeatureData*>& OutActivePluginFeatureDatas)
{
    static auto CurrentState = (int32)Anchor::Offset(TEXT("UGameFeaturePluginStateMachine::CurrentState"))
        .String(TEXT(" - Adding preload object: '%s'"))
        .Caller()
        .Find("call 0x*; mov rcx, rax; lea rdx, [*]; call 0x*; mov rsi, [*]")
        .Call(2)
        .Find("cmp dword ptr [*+0x*], 0x*")
        .Disp();

    static auto Active = (int32)Anchor::Offset(TEXT("EGameFeaturePluginState::Active"))
        .String(TEXT(" - Adding preload object: '%s'"))
        .Caller()
        .Find("call 0x*; mov rcx, rax; lea rdx, [*]; call 0x*; mov rsi, [*]")
        .Call(2)
        .Find("cmp dword ptr [*+0x*], 0x*")
        .Imm();

    static auto GameFeatureData = (int32)Anchor::Offset(TEXT("UGameFeaturePluginStateMachine::StateProperties.GameFeatureData"))
        .String(TEXT(" - Adding preload object: '%s'"))
        .Caller()
        .Find("call 0x*; mov rcx, rax; lea rdx, [*]; call 0x*; mov rsi, [*]")
        .Call(2)
        .Find("cmp dword ptr [*+0x*], 0x*")
        .Find("jz 0x*")
        .Target()
        .Find("mov *, [*+0x*]")
        .Disp();

    static auto GameFeaturePluginStateMachines = GetClass()->FindPropertyByName(FName(TEXT("GameFeaturePluginStateMachines")));

    if (CurrentState <= 0 || GameFeatureData <= 0 || !GameFeaturePluginStateMachines)
        return;

    for (auto& StateMachinePair : *GameFeaturePluginStateMachines->ContainerPtrToValuePtr<TMap<FString, UObject*>>(this))
    {
        auto StateMachine = StateMachinePair.Value;

        if (!StateMachine || *(int32*)((uint8_t*)StateMachine + CurrentState) != Active)
            continue;

        if (auto FeatureData = *(const UGameFeatureData**)((uint8_t*)StateMachine + GameFeatureData))
            OutActivePluginFeatureDatas.Add(FeatureData);
    }
}
