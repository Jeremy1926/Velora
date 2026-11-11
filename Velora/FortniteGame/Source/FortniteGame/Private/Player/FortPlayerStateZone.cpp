#include "pch.h"
#include "Player/FortPlayerStateZone.h"
#include "Abilities/FortAbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AFortPlayerStateZone::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerStateZone"));

    return Class;
}

void AFortPlayerStateZone::TickSimulatedAttributes(UWorld* World)
{
    auto GameState = World ? World->GetGameState() : nullptr;

    if (!GameState)
        return;

    for (auto PlayerState : GameState->GetPlayerArray())
    {
        if (auto PlayerStateZone = Cast<AFortPlayerStateZone>(PlayerState))
            PlayerStateZone->UpdateSimulatedAttributes();
    }
}

void AFortPlayerStateZone::UpdateSimulatedAttributes()
{
    static auto SimulatedAttributes = StaticClass()->FindPropertyByName(FName(TEXT("SimulatedAttributes")));
    static auto SimulatedAttributeArray = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.SimulatedAttributeArray"));
    static auto TrackedAttributes = SimulatedAttributeArray ? SimulatedAttributeArray->FindPropertyByName(FName(TEXT("TrackedAttributes"))) : nullptr;
    static auto OwningPlayerState = SimulatedAttributeArray ? SimulatedAttributeArray->FindPropertyByName(FName(TEXT("OwningPlayerState"))) : nullptr;
    static auto FortAttributeSet = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAttributeSet"));
    static auto GetSimulatedAttributes = Anchor::VirtualFunction<void (*)(UAttributeSet*, TArray<FSimulatedAttributeEntry>&)>(TEXT("UFortAttributeSet::GetSimulatedAttributes"))
        .Virtual(TEXT("/Script/FortniteGame.Default__FortGliderPackSet"), [](const Anchor::FCode& Entry) { return Entry.Has("movaps xmm2, xmm6; lea rcx, [rsp+0x*]; mov rdx, rax; call 0x*; mov rdx, rax; mov rcx, rdi; call 0x*"); })
        .Optional()
        .Get();

    if (!SimulatedAttributes || !TrackedAttributes || !FortAttributeSet || !GetSimulatedAttributes)
        return;

    auto AbilitySystemComponent = GetAbilitySystemComponent();

    if (!AbilitySystemComponent)
        return;

    static auto SpawnedAttributes = AbilitySystemComponent->GetClass()->FindPropertyByName(FName(TEXT("SpawnedAttributes")));

    if (!SpawnedAttributes)
        return;

    auto Array = SimulatedAttributes->ContainerPtrToValuePtr<FFastArraySerializer>(this);
    auto& Tracked = *TrackedAttributes->ContainerPtrToValuePtr<TArray<FSimulatedAttributeEntry>>(Array);
    TArray<FSimulatedAttributeEntry> Current;
    bool bChanged = false;

    if (OwningPlayerState)
        *OwningPlayerState->ContainerPtrToValuePtr<AFortPlayerStateZone*>(Array) = this;

    for (auto AttributeSet : *SpawnedAttributes->ContainerPtrToValuePtr<TArray<UAttributeSet*>>(AbilitySystemComponent))
    {
        if (AttributeSet && AttributeSet->IsA(FortAttributeSet))
            GetSimulatedAttributes(AttributeSet, Current);
    }

    for (int32 Index = Tracked.Num() - 1; Index >= 0; Index--)
    {
        bool bStillTracked = false;

        for (auto& Entry : Current)
            bStillTracked |= Entry.Attribute == Tracked[Index].Attribute;

        if (bStillTracked)
            continue;

        Tracked.RemoveAt(Index);
        Array->MarkArrayDirty();
        bChanged = true;
    }

    for (auto& Entry : Current)
    {
        int32 TrackedIndex = INDEX_NONE;

        for (int32 Index = 0; Index < Tracked.Num(); Index++)
        {
            if (Tracked[Index].Attribute == Entry.Attribute)
                TrackedIndex = Index;
        }

        if (TrackedIndex == INDEX_NONE)
        {
            TrackedIndex = Tracked.Add(Entry);
            Array->MarkItemDirty(Tracked[TrackedIndex]);
            bChanged = true;
        }
        else if (Tracked[TrackedIndex].CurrentValue != Entry.CurrentValue)
        {
            Tracked[TrackedIndex].CurrentValue = Entry.CurrentValue;
            Array->MarkItemDirty(Tracked[TrackedIndex]);
            bChanged = true;
        }
    }

    if (bChanged)
        ForceNetUpdate();
}

void AFortPlayerStateZone::BeginSpectating(AFortPlayerStateZone* PlayerToSpectate)
{
    static auto BeginSpectating = Anchor::Function<void (*)(AFortPlayerStateZone*, AFortPlayerStateZone*)>(TEXT("AFortPlayerStateZone::BeginSpectating"))
        .String(TEXT("BeginSpectating: Attempting to spectate self!"))
        .Start()
        .Get();

    BeginSpectating(this, PlayerToSpectate);
}
