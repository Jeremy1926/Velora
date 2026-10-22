#include "pch.h"
#include "Building/BuildingGameplayActorTransientQuest.h"
#include "Athena/ControllerComponents/FortControllerComponent_TransientQuests.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Pawns/FortPlayerPawn.h"
#include "Player/FortPlayerController.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"
#include "Detour.h"

struct BuildingGameplayActorTransientQuest_eventMulticastSetInteractionText_Parms
{
    const AFortPlayerPawn* PawnInTriggerVolume;
    bool bBountyAvailable;
    bool bBountyInProgress;
};

struct FPendingInteractionTextUpdate
{
    TWeakObjectPtr<ABuildingGameplayActorTransientQuest> TransientQuest;
    float NextUpdateTime;
};

static TArray<FPendingInteractionTextUpdate> PendingInteractionTextUpdates;

UClass* ABuildingGameplayActorTransientQuest::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingGameplayActorTransientQuest"));

    return Class;
}

void ABuildingGameplayActorTransientQuest::StartUpdatingInteractionText(const AActor* TriggeredActor)
{
    auto Pawn = Cast<AFortPlayerPawn>(TriggeredActor);

    if (!Pawn || !HasAuthority())
        return;

    GetPawnsInTriggerVolume().AddUnique(Pawn);

    auto bUpdating = false;

    for (auto& Update : PendingInteractionTextUpdates)
        bUpdating |= Update.TransientQuest.Get() == this;

    if (!bUpdating)
        PendingInteractionTextUpdates.Add(FPendingInteractionTextUpdate{ this, GetWorld()->GetTimeSeconds() + GetInteractionTextRefreshRate() });

    ServerUpdateInteractionText_Implementation();
}

void ABuildingGameplayActorTransientQuest::StopUpdatingInteractionText(const AActor* TriggeredActor)
{
    auto Pawn = Cast<AFortPlayerPawn>(TriggeredActor);

    if (!Pawn || !HasAuthority())
        return;

    GetPawnsInTriggerVolume().RemoveSingle(Pawn);

    if (GetPawnsInTriggerVolume().Num())
        return;

    for (int32 Index = PendingInteractionTextUpdates.Num() - 1; Index >= 0; Index--)
    {
        if (PendingInteractionTextUpdates[Index].TransientQuest.Get() == this)
            PendingInteractionTextUpdates.RemoveAt(Index);
    }
}

void ABuildingGameplayActorTransientQuest::ServerUpdateInteractionText_Implementation()
{
    DETOUR(ABuildingGameplayActorTransientQuest, ServerUpdateInteractionText_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.BuildingGameplayActorTransientQuest:ServerUpdateInteractionText"))
        .Find("jmp [*+0x*]")
        .LogSlot(TEXT("ABuildingGameplayActorTransientQuest::ServerUpdateInteractionText_Implementation"))
        .IsVirtual(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorTransientQuest"), "ret*")
        .Optional()
        .InVTable(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorTransientQuest")));

    for (auto Pawn : GetPawnsInTriggerVolume())
    {
        auto PlayerController = Pawn ? Cast<AFortPlayerController>(Pawn->GetController()) : nullptr;
        auto TransientQuests = PlayerController ? PlayerController->FindComponentByClass<UFortControllerComponent_TransientQuests>() : nullptr;

        if (!TransientQuests)
            continue;

        const bool bBountyInProgress = TransientQuests->GetTrackedHunterBountyTarget() != nullptr;
        TArray<AFortPlayerStateAthena*> HumanTargets;
        TArray<AFortPlayerStateAthena*> BotTargets;

        if (!bBountyInProgress)
            UFortControllerComponent_TransientQuests::GetPlayerBountyTargets(PlayerController, GetTargetFilter(), HumanTargets, BotTargets);

        MulticastSetInteractionText(Pawn, HumanTargets.Num() + BotTargets.Num() > 0, bBountyInProgress);
    }
}

void ABuildingGameplayActorTransientQuest::MulticastSetInteractionText(const AFortPlayerPawn* PawnInTriggerVolume, bool bBountyAvailable, bool bBountyInProgress)
{
    static auto MulticastSetInteractionText = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingGameplayActorTransientQuest:MulticastSetInteractionText"));

    BuildingGameplayActorTransientQuest_eventMulticastSetInteractionText_Parms Parms{ PawnInTriggerVolume, bBountyAvailable, bBountyInProgress };

    if (MulticastSetInteractionText)
        CallRemoteFunction(MulticastSetInteractionText, &Parms, nullptr, nullptr);
}

void ABuildingGameplayActorTransientQuest::OnBountyStarted(const TArray<AFortPlayerStateAthena*>& HunterPlayerStates)
{
    auto Multicast_BountyStarted = (UFunction*)StaticFindObject(nullptr, GetClass(), TEXT("Multicast_BountyStarted"));
    auto PlayersToIgnore = GetClass()->FindPropertyByName(FName(TEXT("PlayersToIgnore")));
    auto BountyBoard_Full = GetClass()->FindPropertyByName(FName(TEXT("BountyBoard_Full")));
    auto BountyBoard_Empty = GetClass()->FindPropertyByName(FName(TEXT("BountyBoard_Empty")));
    auto TriggerVolume = GetClass()->FindPropertyByName(FName(TEXT("TriggerVolume")));
    auto ShouldUpdateInteractionWidget = GetClass()->FindPropertyByName(FName(TEXT("ShouldUpdateInteractionWidget")));

    if (!Multicast_BountyStarted || !PlayersToIgnore || !BountyBoard_Full || !BountyBoard_Empty || !TriggerVolume || !ShouldUpdateInteractionWidget)
        return;

    for (auto HunterPlayerState : HunterPlayerStates)
    {
        auto PlayerController = HunterPlayerState ? Cast<AFortPlayerController>(HunterPlayerState->GetOwner()) : nullptr;

        if (!PlayerController)
            continue;

        CallRemoteFunction(Multicast_BountyStarted, &PlayerController, nullptr, nullptr);

        if (auto Pawn = PlayerController->GetMyFortPawn())
        {
            PlayersToIgnore->ContainerPtrToValuePtr<TArray<AActor*>>(this)->Add(Pawn);
            StopUpdatingInteractionText(Pawn);
        }

        if (auto Component = *BountyBoard_Full->ContainerPtrToValuePtr<UPrimitiveComponent*>(this))
            Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        if (auto Component = *BountyBoard_Empty->ContainerPtrToValuePtr<UPrimitiveComponent*>(this))
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        if (auto Component = *TriggerVolume->ContainerPtrToValuePtr<UPrimitiveComponent*>(this))
            Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        *ShouldUpdateInteractionWidget->ContainerPtrToValuePtr<bool>(this) = false;
    }
}

void ABuildingGameplayActorTransientQuest::execStartUpdatingInteractionText(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(ABuildingGameplayActorTransientQuest, execStartUpdatingInteractionText, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.BuildingGameplayActorTransientQuest:StartUpdatingInteractionText"))
        .Is("ret*")
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.BuildingGameplayActorTransientQuest:StartUpdatingInteractionText")));

    AActor* TriggeredActor = nullptr;

    Stack.StepCompiledIn(&TriggeredActor);
    Stack.Code += !!Stack.Code;

    StartUpdatingInteractionText(TriggeredActor);
}

void ABuildingGameplayActorTransientQuest::execStopUpdatingInteractionText(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(ABuildingGameplayActorTransientQuest, execStopUpdatingInteractionText, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.BuildingGameplayActorTransientQuest:StopUpdatingInteractionText"))
        .Is("ret*")
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.BuildingGameplayActorTransientQuest:StopUpdatingInteractionText")));

    AActor* TriggeredActor = nullptr;

    Stack.StepCompiledIn(&TriggeredActor);
    Stack.Code += !!Stack.Code;

    StopUpdatingInteractionText(TriggeredActor);
}

void ABuildingGameplayActorTransientQuest::TickInteractionTextUpdates()
{
    for (int32 Index = PendingInteractionTextUpdates.Num() - 1; Index >= 0; Index--)
    {
        auto TransientQuest = PendingInteractionTextUpdates[Index].TransientQuest.Get();

        if (!TransientQuest)
        {
            PendingInteractionTextUpdates.RemoveAt(Index);
            continue;
        }

        if (TransientQuest->GetWorld()->GetTimeSeconds() < PendingInteractionTextUpdates[Index].NextUpdateTime)
            continue;

        PendingInteractionTextUpdates[Index].NextUpdateTime += TransientQuest->GetInteractionTextRefreshRate();

        TransientQuest->ServerUpdateInteractionText_Implementation();
    }
}

TArray<AFortPlayerPawn*>& ABuildingGameplayActorTransientQuest::GetPawnsInTriggerVolume()
{
    static auto PawnsInTriggerVolume = StaticClass()->FindPropertyByName(FName(TEXT("PawnsInTriggerVolume")));

    return *PawnsInTriggerVolume->ContainerPtrToValuePtr<TArray<AFortPlayerPawn*>>(this);
}

float ABuildingGameplayActorTransientQuest::GetInteractionTextRefreshRate() const
{
    static auto InteractionTextRefreshRate = StaticClass()->FindPropertyByName(FName(TEXT("InteractionTextRefreshRate")));

    return *InteractionTextRefreshRate->ContainerPtrToValuePtr<float>(this);
}

const FGameplayTagQuery& ABuildingGameplayActorTransientQuest::GetTargetFilter() const
{
    static auto TargetFilter = StaticClass()->FindPropertyByName(FName(TEXT("TargetFilter")));

    return *TargetFilter->ContainerPtrToValuePtr<FGameplayTagQuery>(this);
}
