#include "pch.h"
#include "Athena/ControllerComponents/FortControllerComponent_TransientQuests.h"
#include "AI/FortAthenaAIBotController.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Quests/FortQuestItemDefinition.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"
#include "Detour.h"

struct FPendingThreatLevelUpdate
{
    TWeakObjectPtr<UFortControllerComponent_TransientQuests> Component;
    float MediumThreatRange;
    float HighThreatRange;
    float ThreatRefreshRate;
    float NextUpdateTime;
    EPlayerBountyThreatLevel PlayerBountyThreatLevel;
};

static TArray<FPendingThreatLevelUpdate> PendingThreatLevelUpdates;

UClass* UFortControllerComponent_TransientQuests::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerComponent_TransientQuests"));

    return Class;
}

void UFortControllerComponent_TransientQuests::GrantTransientQuest(const UFortQuestItemDefinition* QuestDefToGrant)
{
    static auto GrantTransientQuest = Anchor::Function<void (*)(UFortControllerComponent_TransientQuests*, const UFortQuestItemDefinition*, const UFortQuestItemDefinition*)>(TEXT("UFortControllerComponent_TransientQuests::GrantTransientQuest"))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerBountyGameplayAbility:BountyPoached"))
        .Find("jmp 0x*")
        .Target()
        .Find("lea rdx, [rsp+*]; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea rax, [0x*]")
        .Target()
        .Find("mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerBountyGameplayAbility:BountyPoached"))
        .Find("lea rcx, [rsp+0x*]; call 0x*; mov rdx, rax; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("lea rax, [0x*]")
        .Target()
        .Find("mov rdx, *; mov rcx, *; call 0x*; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortControllerComponent_TransientQuests:GrantTransientQuest"))
        .Find("add [*+0x20], *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (QuestDefToGrant)
        GrantTransientQuest(this, QuestDefToGrant, nullptr);
}

void UFortControllerComponent_TransientQuests::GrantTransientQuest(TSoftObjectPtr<UFortQuestItemDefinition> QuestDefToGrantPtr)
{
    auto QuestDefToGrant = QuestDefToGrantPtr.LoadSynchronous();

    if (!QuestDefToGrant)
        return;

    GrantTransientQuest(QuestDefToGrant);
    OnTransientQuestGranted(QuestDefToGrant);
}

void UFortControllerComponent_TransientQuests::OnTransientQuestGranted(const UFortQuestItemDefinition* GrantedQuestDef)
{
    static auto OnTransientQuestGranted = Anchor::Function<void (*)(UFortControllerComponent_TransientQuests*, const UFortQuestItemDefinition*)>(TEXT("UFortControllerComponent_TransientQuests::OnTransientQuestGranted"))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerBountyGameplayAbility:BountyPoached"))
        .Find("jmp 0x*")
        .Target()
        .Find("lea rdx, [rsp+*]; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea rax, [0x*]")
        .Target()
        .Find("mov rdx, *; mov rcx, *; call 0x*; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerBountyGameplayAbility:BountyPoached"))
        .Find("lea rcx, [rsp+0x*]; call 0x*; mov rdx, rax; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("lea rax, [0x*]")
        .Target()
        .Find("mov rdx, *; mov rcx, *; call 0x*; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerBountyGameplayAbility:BountyPoached"))
        .Find("jmp 0x*")
        .Target()
        .Find("lea rcx, [0x*]")
        .Target()
        .Find("xor r8d, r8d; mov rdx, *; mov rcx, *; call 0x*; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerBountyGameplayAbility:BountyPoached"))
        .Find("jmp 0x*")
        .Target()
        .Find("lea rcx, [rsp+0x*]; call 0x*; mov rdx, rax; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("lea rax, [0x*]")
        .Target()
        .Find("xor r8d, r8d; mov rdx, *; mov rcx, *; call 0x*; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Get();

    if (GrantedQuestDef)
        OnTransientQuestGranted(this, GrantedQuestDef);
}

void UFortControllerComponent_TransientQuests::StartPlayerBountyThreatLevelUpdates(float MediumThreatRange, float HighThreatRange, float ThreatRefreshRate)
{
    static auto MinimumThreatLevelRefreshRate = Anchor::Data<float*>(TEXT("FortTransientQuestsComponentCVars::MinimumThreatLevelRefreshRate"))
        .String(TEXT("Fort.Bounty.MinimumThreatLevelRefreshRate"))
        .Last("lea r8, [0x*]")
        .Target()
        .Get();
    static auto MaximumThreatLevelRefreshRate = Anchor::Data<float*>(TEXT("FortTransientQuestsComponentCVars::MaximumThreatLevelRefreshRate"))
        .String(TEXT("Fort.Bounty.MaximumThreatLevelRefreshRate"))
        .Last("lea r8, [0x*]")
        .Target()
        .Get();

    auto Controller = Cast<AController>(GetOwner());
    auto Pawn = Controller ? Cast<AFortPlayerPawn>(Controller->GetPawn()) : nullptr;

    if (!Pawn || !Pawn->HasAuthority() || !MinimumThreatLevelRefreshRate || !MaximumThreatLevelRefreshRate)
        return;

    for (int32 Index = PendingThreatLevelUpdates.Num() - 1; Index >= 0; Index--)
    {
        if (PendingThreatLevelUpdates[Index].Component.Get() == this)
            PendingThreatLevelUpdates.RemoveAt(Index);
    }

    const float RefreshRate = ThreatRefreshRate < *MinimumThreatLevelRefreshRate ? *MinimumThreatLevelRefreshRate : FMath::Min(*MaximumThreatLevelRefreshRate, ThreatRefreshRate);

    PendingThreatLevelUpdates.Add(FPendingThreatLevelUpdate{ this, MediumThreatRange, HighThreatRange, RefreshRate, Pawn->GetWorld()->GetTimeSeconds() + RefreshRate, EPlayerBountyThreatLevel::Low });

    UpdatePlayerBountyThreatLevel(MediumThreatRange, HighThreatRange);
}

void UFortControllerComponent_TransientQuests::StopPlayerBountyThreatLevelUpdates()
{
    DETOUR(UFortControllerComponent_TransientQuests, StopPlayerBountyThreatLevelUpdates, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortControllerComponent_TransientQuests:StopPlayerBountyThreatLevelUpdates"))
        .Find("jmp 0x*")
        .Target()
        .Optional()
        .Inline());

    for (int32 Index = PendingThreatLevelUpdates.Num() - 1; Index >= 0; Index--)
    {
        if (PendingThreatLevelUpdates[Index].Component.Get() == this)
            PendingThreatLevelUpdates.RemoveAt(Index);
    }

    Original();
}

void UFortControllerComponent_TransientQuests::UpdatePlayerBountyThreatLevel(float MediumThreatRange, float HighThreatRange)
{
    auto Controller = Cast<AController>(GetOwner());
    auto PlayerState = Controller ? Cast<AFortPlayerStateAthena>(Controller->GetPlayerState()) : nullptr;
    auto Pawn = Controller ? Cast<AFortPlayerPawnAthena>(Controller->GetPawn()) : nullptr;
    FPendingThreatLevelUpdate* PendingUpdate = nullptr;

    for (auto& Update : PendingThreatLevelUpdates)
    {
        if (Update.Component.Get() == this)
            PendingUpdate = &Update;
    }

    if (!PlayerState || !Pawn || !Pawn->HasAuthority() || !PendingUpdate)
        return;

    const FVector Location = Pawn->GetActorLocation();
    AFortPlayerStateAthena* ClosestBountyHunter = nullptr;
    float ClosestDistanceSquared = 0.0f;

    for (auto BountyHunter : GetTrackedBountyHunters())
    {
        auto BountyHunterController = BountyHunter ? Cast<AController>(BountyHunter->GetOwner()) : nullptr;
        auto BountyHunterPawn = BountyHunterController ? Cast<AFortPlayerPawnAthena>(BountyHunterController->GetPawn()) : nullptr;

        if (!BountyHunterPawn)
            continue;

        const float DistanceSquared = (BountyHunterPawn->GetActorLocation() - Location).SizeSquared();

        if (!ClosestBountyHunter || DistanceSquared < ClosestDistanceSquared)
        {
            ClosestBountyHunter = BountyHunter;
            ClosestDistanceSquared = DistanceSquared;
        }
    }

    auto PlayerBountyThreatLevel = EPlayerBountyThreatLevel::Low;

    if (ClosestBountyHunter && ClosestDistanceSquared < HighThreatRange * HighThreatRange)
        PlayerBountyThreatLevel = EPlayerBountyThreatLevel::High;
    else if (ClosestBountyHunter && ClosestDistanceSquared < MediumThreatRange * MediumThreatRange)
        PlayerBountyThreatLevel = EPlayerBountyThreatLevel::Medium;

    if (PendingUpdate->PlayerBountyThreatLevel == PlayerBountyThreatLevel)
        return;

    PendingUpdate->PlayerBountyThreatLevel = PlayerBountyThreatLevel;

    ClientBroadcastOnPlayerBountyThreatLevelUpdated(PlayerBountyThreatLevel);
}

void UFortControllerComponent_TransientQuests::ClientBroadcastOnPlayerBountyThreatLevelUpdated(EPlayerBountyThreatLevel NewPlayerBountyThreatLevel)
{
    static auto ClientBroadcastOnPlayerBountyThreatLevelUpdated = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerComponent_TransientQuests:ClientBroadcastOnPlayerBountyThreatLevelUpdated"));

    if (ClientBroadcastOnPlayerBountyThreatLevelUpdated)
        CallRemoteFunction(ClientBroadcastOnPlayerBountyThreatLevelUpdated, &NewPlayerBountyThreatLevel, nullptr, nullptr);
}

void UFortControllerComponent_TransientQuests::TickPlayerBountyThreatLevelUpdates()
{
    for (int32 Index = PendingThreatLevelUpdates.Num() - 1; Index >= 0; Index--)
    {
        auto Component = PendingThreatLevelUpdates[Index].Component.Get();
        auto Owner = Component ? Component->GetOwner() : nullptr;

        if (!Owner)
        {
            PendingThreatLevelUpdates.RemoveAt(Index);
            continue;
        }

        if (Owner->GetWorld()->GetTimeSeconds() < PendingThreatLevelUpdates[Index].NextUpdateTime)
            continue;

        PendingThreatLevelUpdates[Index].NextUpdateTime += PendingThreatLevelUpdates[Index].ThreatRefreshRate;

        Component->UpdatePlayerBountyThreatLevel(PendingThreatLevelUpdates[Index].MediumThreatRange, PendingThreatLevelUpdates[Index].HighThreatRange);
    }
}

void UFortControllerComponent_TransientQuests::GetPlayerBountyTargets(const AController* Hunter, const FGameplayTagQuery& TargetFilter, TArray<AFortPlayerStateAthena*>& OutHumanTargets, TArray<AFortPlayerStateAthena*>& OutBotTargets)
{
    static auto GameplayTagAssetInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayTags.GameplayTagAssetInterface"));

    auto HunterPlayerState = Hunter ? Cast<AFortPlayerStateAthena>(Hunter->GetPlayerState()) : nullptr;
    auto GameMode = HunterPlayerState ? Cast<AFortGameModeAthena>(Hunter->GetWorld()->GetAuthGameMode()) : nullptr;

    if (!GameMode)
        return;

    auto GetPlayerBountyTarget = [&](AController* Target) -> AFortPlayerStateAthena*
    {
        auto TargetPlayerState = Target && Target != Hunter ? Cast<AFortPlayerStateAthena>(Target->GetPlayerState()) : nullptr;
        auto TargetPawn = TargetPlayerState ? Target->GetPawn() : nullptr;
        auto TagAssetInterface = TargetPawn ? (IGameplayTagAssetInterface*)TargetPawn->GetInterfaceAddress(GameplayTagAssetInterface) : nullptr;
        FGameplayTagContainer OwnedTags;

        if (!TagAssetInterface || TargetPlayerState->GetTeamIndex() == HunterPlayerState->GetTeamIndex())
            return nullptr;

        TagAssetInterface->GetOwnedGameplayTags(OwnedTags);

        return TargetFilter.IsEmpty() || TargetFilter.Matches(OwnedTags) ? TargetPlayerState : nullptr;
    };

    for (auto PlayerController : GameMode->GetAlivePlayers())
    {
        if (auto Target = GetPlayerBountyTarget((AController*)PlayerController))
            OutHumanTargets.Add(Target);
    }

    for (auto BotController : GameMode->GetAliveBots())
    {
        if (auto Target = GetPlayerBountyTarget((AController*)BotController))
            OutBotTargets.Add(Target);
    }
}

AFortPlayerStateAthena*& UFortControllerComponent_TransientQuests::GetTrackedHunterBountyTarget()
{
    static auto TrackedHunterBountyTarget = StaticClass()->FindPropertyByName(FName(TEXT("TrackedHunterBountyTarget")));

    return *TrackedHunterBountyTarget->ContainerPtrToValuePtr<AFortPlayerStateAthena*>(this);
}

AFortPlayerStateAthena*& UFortControllerComponent_TransientQuests::GetTrackedProtectorBountyTarget()
{
    static auto TrackedProtectorBountyTarget = StaticClass()->FindPropertyByName(FName(TEXT("TrackedProtectorBountyTarget")));

    return *TrackedProtectorBountyTarget->ContainerPtrToValuePtr<AFortPlayerStateAthena*>(this);
}

TArray<AFortPlayerStateAthena*>& UFortControllerComponent_TransientQuests::GetTrackedBountyHunters()
{
    static auto TrackedBountyHunters = StaticClass()->FindPropertyByName(FName(TEXT("TrackedBountyHunters")));

    return *TrackedBountyHunters->ContainerPtrToValuePtr<TArray<AFortPlayerStateAthena*>>(this);
}
