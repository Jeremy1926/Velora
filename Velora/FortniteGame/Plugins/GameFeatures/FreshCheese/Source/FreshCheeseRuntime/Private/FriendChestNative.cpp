#include "pch.h"
#include "FriendChestNative.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Items/FortInventory.h"
#include "Items/FortLootPackage.h"
#include "Items/FortLootTier.h"
#include "Items/FortWorldItemDefinition.h"
#include "Player/FortPlayerController.h"
#include "FortKismetLibrary.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Scanner.h"
#include "Detour.h"

struct FFriendChestPendingActivation
{
    TWeakObjectPtr<AFriendChestNative> Chest;
    TWeakObjectPtr<AFortPlayerPawnAthena> Player;
    float Time;
};

static TArray<FFriendChestPendingActivation> PendingActivations;

UClass* AFriendChestNative::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FreshCheeseRuntime.FriendChestNative"));

    return Class;
}

void AFriendChestNative::execOnTriggerComponentBeginOverlap(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFriendChestNative, execOnTriggerComponentBeginOverlap, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FreshCheeseRuntime.FriendChestNative:OnTriggerComponentBeginOverlap"))
        .Optional()
        .InExec(TEXT("/Script/FreshCheeseRuntime.FriendChestNative:OnTriggerComponentBeginOverlap")));

    UPrimitiveComponent* OverlappedComponent = nullptr;
    AActor* OtherActor = nullptr;
    UPrimitiveComponent* OtherComp = nullptr;
    int32 OtherBodyIndex = 0;
    uint32 bFromSweep = 0;
    FHitResult SweepResultTemp;

    Stack.StepCompiledIn(&OverlappedComponent);
    Stack.StepCompiledIn(&OtherActor);
    Stack.StepCompiledIn(&OtherComp);
    Stack.StepCompiledIn(&OtherBodyIndex);
    Stack.StepCompiledIn(&bFromSweep);
    const FHitResult& SweepResult = Stack.StepCompiledInRef<FHitResult>(&SweepResultTemp);
    Stack.Code += !!Stack.Code;

    OnTriggerComponentBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, !!bFromSweep, SweepResult);
}

void AFriendChestNative::execOnTriggerComponentEndOverlap(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFriendChestNative, execOnTriggerComponentEndOverlap, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FreshCheeseRuntime.FriendChestNative:OnTriggerComponentEndOverlap"))
        .Optional()
        .InExec(TEXT("/Script/FreshCheeseRuntime.FriendChestNative:OnTriggerComponentEndOverlap")));

    UPrimitiveComponent* OverlappedComponent = nullptr;
    AActor* OtherActor = nullptr;
    UPrimitiveComponent* OtherComp = nullptr;
    int32 OtherBodyIndex = 0;

    Stack.StepCompiledIn(&OverlappedComponent);
    Stack.StepCompiledIn(&OtherActor);
    Stack.StepCompiledIn(&OtherComp);
    Stack.StepCompiledIn(&OtherBodyIndex);
    Stack.Code += !!Stack.Code;

    OnTriggerComponentEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}

void AFriendChestNative::execRollInstancedLootForSquadMembers(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFriendChestNative, execRollInstancedLootForSquadMembers, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FreshCheeseRuntime.FriendChestNative:RollInstancedLootForSquadMembers"))
        .Optional()
        .InExec(TEXT("/Script/FreshCheeseRuntime.FriendChestNative:RollInstancedLootForSquadMembers")));

    AGameModeBase* GameMode = nullptr;
    uint8 SquadId = 0;

    Stack.StepCompiledIn(&GameMode);
    Stack.StepCompiledIn(&SquadId);
    Stack.Code += !!Stack.Code;

    RollInstancedLootForSquadMembers(GameMode, SquadId);
}

void AFriendChestNative::OnTriggerComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    auto Player = Cast<AFortPlayerPawnAthena>(OtherActor);

    if (!HasAuthority() || !Player)
        return;

    for (auto& Activation : PendingActivations)
    {
        if (Activation.Chest.Get() == this && Activation.Player.Get() == Player)
            return;
    }

    const float ActivationDelay = GetTrigerActivationDelayCurve().GetValueAtLevel(0.0f);

    if (ActivationDelay <= 0.0f)
    {
        PlayerAdded(Player);
        return;
    }

    PendingActivations.Add(FFriendChestPendingActivation{ this, Player, GetWorld()->GetTimeSeconds() + ActivationDelay });
}

void AFriendChestNative::OnTriggerComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    auto Player = Cast<AFortPlayerPawnAthena>(OtherActor);

    if (!HasAuthority() || !Player)
        return;

    for (int32 Index = 0; Index < PendingActivations.Num(); Index++)
    {
        if (PendingActivations[Index].Chest.Get() == this && PendingActivations[Index].Player.Get() == Player)
        {
            PendingActivations.RemoveAt(Index);
            return;
        }
    }

    PlayerRemoved(Player);
}

void AFriendChestNative::TickActivationTimers()
{
    TArray<FFriendChestPendingActivation> ExpiredActivations;

    for (int32 Index = PendingActivations.Num() - 1; Index >= 0; Index--)
    {
        auto Chest = PendingActivations[Index].Chest.Get();
        auto Player = PendingActivations[Index].Player.Get();

        if (Chest && Player && Chest->GetWorld()->GetTimeSeconds() < PendingActivations[Index].Time)
            continue;

        if (Chest && Player)
            ExpiredActivations.Add(PendingActivations[Index]);

        PendingActivations.RemoveAt(Index);
    }

    for (auto& Activation : ExpiredActivations)
    {
        auto Chest = Activation.Chest.Get();
        auto Player = Activation.Player.Get();

        if (Chest && Player)
            Chest->PlayerAdded(Player);
    }
}

void AFriendChestNative::RollInstancedLootForSquadMembers(AGameModeBase* GameMode, uint8 SquadId)
{
    TArray<AFortPlayerStateAthena*> SquadMembers;

    GetAssociatedSquadMembers(false, false, false, SquadMembers);

    FGameplayTagContainer GameplayTags;
    GameplayTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("FreshCheese.BonusState.Fresh")), false));

    for (auto SquadMember : SquadMembers)
    {
        auto PlayerController = SquadMember ? Cast<AFortPlayerController>(SquadMember->GetOwner()) : nullptr;
        FName LootTierKey(NAME_None);
        int32 LootTier = -1;
        TArray<FFortItemEntry> LootToDrop;

        if (!PlayerController || !UFortLootTier::ChooseUnquotaedLootTierKey(LootTierKey, LootTier, GetInstancedLootTierGroup(), -1, GameplayTags, SquadMembers.Num()))
            continue;

        if (!UFortLootPackage::PickLootDrops(LootToDrop, -1, LootTierKey, GameplayTags, true, false))
            continue;

        for (int32 Index = 0; Index < LootToDrop.Num(); Index++)
        {
            if (auto ItemDefinition = Cast<UFortWorldItemDefinition>(LootToDrop[Index].GetItemDefinition()))
                UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, ItemDefinition, FMath::Max(LootToDrop[Index].GetCount(), 1));
        }
    }
}

void AFriendChestNative::GetAssociatedSquadMembers(bool bIncludeDead, bool bIncludeResurectable, bool bIncludeDisconnected, TArray<AFortPlayerStateAthena*>& OutSquadMembers) const
{
    static auto GetAssociatedSquadMembers = Anchor::Function<void (*)(const AFriendChestNative*, bool, bool, bool, TArray<AFortPlayerStateAthena*>&)>(TEXT("AFriendChestNative::GetAssociatedSquadMembers"))
        .Native(TEXT("/Script/FreshCheeseRuntime.FriendChestNative:GetAssociatedSquadMembers"))
        .Get();

    GetAssociatedSquadMembers(this, bIncludeDead, bIncludeResurectable, bIncludeDisconnected, OutSquadMembers);
}

void AFriendChestNative::PlayerAdded(AFortPlayerPawnAthena* Player)
{
    struct FriendChestNative_eventPlayerAdded_Parms
    {
        AFortPlayerPawnAthena* Player;
    };

    FriendChestNative_eventPlayerAdded_Parms Parms{ Player };

    ProcessEvent(FindFunctionChecked(FName(TEXT("PlayerAdded"))), &Parms);
}

void AFriendChestNative::PlayerRemoved(AFortPlayerPawnAthena* Player)
{
    struct FriendChestNative_eventPlayerRemoved_Parms
    {
        AFortPlayerPawnAthena* Player;
    };

    FriendChestNative_eventPlayerRemoved_Parms Parms{ Player };

    ProcessEvent(FindFunctionChecked(FName(TEXT("PlayerRemoved"))), &Parms);
}

FScalableFloat& AFriendChestNative::GetTrigerActivationDelayCurve()
{
    static auto TrigerActivationDelayCurve = StaticClass()->FindPropertyByName(FName(TEXT("TrigerActivationDelayCurve")));

    return *TrigerActivationDelayCurve->ContainerPtrToValuePtr<FScalableFloat>(this);
}

FName AFriendChestNative::GetInstancedLootTierGroup() const
{
    static auto InstancedLootTierGroup = StaticClass()->FindPropertyByName(FName(TEXT("InstancedLootTierGroup")));

    return InstancedLootTierGroup->GetPropertyValue_InContainer<FName>(this);
}
